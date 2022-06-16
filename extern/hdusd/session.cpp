/* SPDX-License-Identifier: Apache-2.0
 * Copyright 2011-2022 Blender Foundation */

#include <GL/glew.h>

#include <pxr/usd/usd/stage.h>
#include <pxr/base/gf/camera.h>
#include <pxr/base/plug/registry.h>
#include <pxr/imaging/glf/drawTarget.h>
#include <pxr/usd/usdGeom/camera.h>
#include <pxr/usdImaging/usdImagingGL/engine.h>
#include <pxr/usdImaging/usdImagingGL/renderParams.h>
#include <pxr/usdImaging/usdAppUtils/camera.h>

#include "session.h"
#include "usd.h"
#include "view_settings.h"

#define GLOG_NO_ABBREVIATED_SEVERITIES
#include "glog/logging.h"

namespace hdusd {

BlenderSession::BlenderSession(BL::RenderEngine &b_engine)
    : b_engine(b_engine)
{
}

BlenderSession::~BlenderSession()
{
}

static PyObject *create_func(PyObject * /*self*/, PyObject *args)
{
  DLOG(INFO) << "create_func";
  PyObject *pyengine;
  if (!PyArg_ParseTuple(args, "O", &pyengine)) {
    Py_RETURN_NONE;
  }

  PointerRNA engineptr;
  RNA_pointer_create(NULL, &RNA_RenderEngine, (void *)PyLong_AsVoidPtr(pyengine), &engineptr);
  BL::RenderEngine engine(engineptr);

  /* create session */
  BlenderSession *session = new BlenderSession(engine);

  return PyLong_FromVoidPtr(session);
}

static PyObject *reset_func(PyObject * /*self*/, PyObject *args)
{
  DLOG(INFO) << "reset_func";
  PyObject *pysession, *pydata, *pydepsgraph;
  int stageId = 0;
  if (!PyArg_ParseTuple(args, "OOOi", &pysession, &pydata, &pydepsgraph, &stageId)) {
    Py_RETURN_NONE;
  }

  BlenderSession *session = (BlenderSession *)PyLong_AsVoidPtr(pysession);

  PointerRNA dataptr;
  RNA_main_pointer_create((Main *)PyLong_AsVoidPtr(pydata), &dataptr);
  BL::BlendData data(dataptr);

  PointerRNA depsgraphptr;
  RNA_pointer_create(NULL, &RNA_Depsgraph, (ID *)PyLong_AsVoidPtr(pydepsgraph), &depsgraphptr);
  BL::Depsgraph depsgraph(depsgraphptr);

  session->stage = stageCache->Find(pxr::UsdStageCache::Id::FromLongInt(stageId));

  Py_RETURN_NONE;
}

static PyObject *free_func(PyObject * /*self*/, PyObject *args)
{
  DLOG(INFO) << "free_func";
  PyObject *pysession;
  if (!PyArg_ParseTuple(args, "O", &pysession)) {
    Py_RETURN_NONE;
  }

  delete (BlenderSession *)PyLong_AsVoidPtr(pysession);
  Py_RETURN_NONE;
}

static PyObject *render_func(PyObject * /*self*/, PyObject *args)
{
  DLOG(INFO) << "render_func";
  PyObject *pysession, *pydepsgraph;

  if (!PyArg_ParseTuple(args, "OO", &pysession, &pydepsgraph)) {
    Py_RETURN_NONE;
  }

  BlenderSession *session = (BlenderSession *)PyLong_AsVoidPtr(pysession);

  if (!session->imagingGLEngine) {
    session->imagingGLEngine = std::make_unique<pxr::UsdImagingGLEngine>();
  }

  pxr::TfToken plugin = pxr::TfToken("HdStormRendererPlugin");

  if (!session->imagingGLEngine->SetRendererPlugin(plugin)) {
    Py_RETURN_NONE;
  }

  PointerRNA depsgraphptr;
  RNA_pointer_create(NULL, &RNA_Depsgraph, (ID *)PyLong_AsVoidPtr(pydepsgraph), &depsgraphptr);
  BL::Depsgraph depsgraph(depsgraphptr);

  BL::Scene b_scene = depsgraph.scene_eval();
  BL::ViewLayer b_view_layer = depsgraph.view_layer();
  string b_render_layer_name = b_view_layer.name();
  vector<vector<float>> border ={{0.0, 0.0}, {1.0, 1.0}};

  if (b_scene.render().use_border()) {
    border = {
      {b_scene.render().border_min_x(),
       b_scene.render().border_min_y()},
      {b_scene.render().border_max_x() - b_scene.render().border_min_x(),
       b_scene.render().border_max_y() - b_scene.render().border_min_y()}
    };
  }

  int screen_width = int(b_scene.render().resolution_x() * b_scene.render().resolution_percentage() / 100);
  int screen_height = int(b_scene.render().resolution_y() * b_scene.render().resolution_percentage() / 100);

  int width = int(screen_width * border[1][0]);
  int height = int(screen_height * border[1][1]);

  pxr::GlfDrawTargetRefPtr draw_target_ptr = pxr::GlfDrawTarget::New(pxr::GfVec2i(width, height));

  draw_target_ptr->Bind();
  draw_target_ptr->AddAttachment("color", GL_RGBA, GL_FLOAT, GL_RGBA);

  pxr::UsdGeomCamera usd_camera = pxr::UsdAppUtilsGetCameraAtPath(session->stage, pxr::SdfPath(pxr::TfMakeValidIdentifier(b_scene.camera().data().name())));
  pxr::UsdTimeCode usd_timecode = pxr::UsdTimeCode(b_scene.frame_current());
  pxr::GfCamera gf_camera = usd_camera.GetCamera(usd_timecode);

  session->imagingGLEngine->SetCameraState(gf_camera.GetFrustum().ComputeViewMatrix(),
                                           gf_camera.GetFrustum().ComputeProjectionMatrix());

  session->imagingGLEngine->SetRenderViewport(pxr::GfVec4d(0, 0, width, height));
  session->imagingGLEngine->SetRendererAov(pxr::HdAovTokens->color);

  session->render_params.frame = usd_timecode;
  session->render_params.clearColor = pxr::GfVec4f(1.0, 1.0, 1.0, 0.0);

  session->imagingGLEngine->Render(session->stage->GetPseudoRoot(), session->render_params);

  BL::RenderResult b_result = session->b_engine.begin_result(0, 0, width, height, b_render_layer_name.c_str(), NULL);
  BL::CollectionRef b_render_passes = b_result.layers[0].passes;

  int channels = 4;
  vector<float> pixels(width * height * channels);

  glReadPixels(0, 0, width, height, GL_RGBA, GL_FLOAT, pixels.data());
  draw_target_ptr->Unbind();

  map<string, vector<float>> render_images{{"Combined", pixels}};
  vector<float> images;

  for (BL::RenderPass b_pass : b_render_passes) {
    map<string, vector<float>>::iterator it_image = render_images.find(b_pass.name());
    vector<float> image = it_image->second;

    if (it_image == render_images.end()) {
      image = vector<float>(width * height * channels);
    }

    if (b_pass.channels() != channels) {
      for (int i = image.size(); i >= b_pass.channels(); i -= b_pass.channels()) {
        image.erase(image.end() - i);
      }
    }

    images.insert(images.end(), image.begin(), image.end());
  }

  for (BL::RenderPass b_pass : b_render_passes) {
    b_pass.rect(images.data());
  }

  session->b_engine.end_result(b_result, false, false, false);

  Py_RETURN_NONE;
}

static PyObject *render_frame_finish_func(PyObject * /*self*/, PyObject *args)
{
  Py_RETURN_NONE;
}

static PyObject *view_update_func(PyObject * /*self*/, PyObject *args)
{
  DLOG(INFO) << "view_update_func";
  Py_RETURN_NONE;
}

static PyObject *view_draw_func(PyObject * /*self*/, PyObject *args)
{
  DLOG(INFO) << "view_draw_func";

  PyObject *pysession, *pydepsgraph, *pycontext, *pyspaceData, *pyregionData;

  if (!PyArg_ParseTuple(args, "OOOOO", &pysession, &pydepsgraph, &pycontext, &pyspaceData, &pyregionData)) {
    Py_RETURN_NONE;
  }

  BlenderSession *session = (BlenderSession *)PyLong_AsVoidPtr(pysession);

  if (!session->imagingGLEngine) {
    session->imagingGLEngine = std::make_unique<pxr::UsdImagingGLEngine>();
  }

  PointerRNA depsgraphptr;
  RNA_pointer_create(NULL, &RNA_Depsgraph, (ID *)PyLong_AsVoidPtr(pydepsgraph), &depsgraphptr);
  BL::Depsgraph depsgraph(depsgraphptr);

  PointerRNA contextptr;
  RNA_pointer_create(NULL, &RNA_Context, (ID *)PyLong_AsVoidPtr(pycontext), &contextptr);
  BL::Context b_context(contextptr);

  BL::Scene b_scene = depsgraph.scene_eval();
  BL::RenderEngine b_engine = session->b_engine;
  
  ViewSettings *view_settings = new ViewSettings(b_context);

  if (view_settings->get_width() * view_settings->get_height() == 0) {
    Py_RETURN_NONE;
  };

  pxr::GfCamera gf_camera = view_settings->export_camera();

  vector<pxr::GfVec4f> clip_planes = gf_camera.GetClippingPlanes();

  for (int i = 0; i < clip_planes.size(); i++) {
    session->render_params.clipPlanes.push_back((pxr::GfVec4d)clip_planes[i]);
  }

  session->imagingGLEngine->SetCameraState(gf_camera.GetFrustum().ComputeViewMatrix(),
                                           gf_camera.GetFrustum().ComputeProjectionMatrix());
  session->imagingGLEngine->SetRenderViewport(pxr::GfVec4d((double)view_settings->border[0][0], (double)view_settings->border[0][1],
                                                  (double)view_settings->border[1][0], (double)view_settings->border[1][1]));

  b_engine.bind_display_space_shader(b_scene);
  
  session->imagingGLEngine->Render(session->stage->GetPseudoRoot(), session->render_params);

  b_engine.unbind_display_space_shader();

  
  ///* Allow Blender to execute other Python scripts. */
  //python_thread_state_save(&session->python_thread_state);

  //session->render(b_depsgraph);

  //python_thread_state_restore(&session->python_thread_state);

  Py_RETURN_NONE;
}

static PyMethodDef methods[] = {
  {"create", create_func, METH_VARARGS, ""},
  {"free", free_func, METH_VARARGS, ""},
  {"render", render_func, METH_VARARGS, ""},
  {"reset", reset_func, METH_VARARGS, ""},
  {"render_frame_finish", render_frame_finish_func, METH_VARARGS, ""},
  {"view_update", view_update_func, METH_VARARGS, ""},
  {"view_draw", view_draw_func, METH_VARARGS, ""},
  {NULL, NULL, 0, NULL},
};

static struct PyModuleDef module = {
  PyModuleDef_HEAD_INIT,
  "session",
  "",
  -1,
  methods,
  NULL,
  NULL,
  NULL,
  NULL,
};

PyObject *session_addPythonSubmodule(PyObject *mod)
{
  PyObject *submodule = PyModule_Create(&module);
  PyModule_AddObject(mod, "session", submodule);
  return submodule;
}

}   // namespace hdusd
