/* SPDX-License-Identifier: Apache-2.0
 * Copyright 2011-2022 Blender Foundation */

#include <epoxy/gl.h>

#include <pxr/base/gf/camera.h>
#include <pxr/imaging/glf/drawTarget.h>
#include <pxr/usd/usdGeom/camera.h>

#include "BLI_math_matrix.h"
#include "DNA_camera_types.h"

#include "glog/logging.h"

#include "viewportEngine.h"
#include "utils.h"

using namespace std;
using namespace pxr;

namespace blender::render::hydra {

struct CameraData {
  CameraData(Object *camera_obj, GfVec2i res, GfVec4f tile);
  CameraData(BL::Context &b_context);

  GfCamera export_gf(GfVec4f tile);

  int mode;
  GfRange1f clip_range;
  float focal_length;
  GfVec2f sensor_size;
  float transform[4][4];
  GfVec2f lens_shift;
  GfVec2f ortho_size;
  tuple<float, float, int> dof_data;
};

struct ViewSettings {
  ViewSettings(BL::Context &b_context);

  int width();
  int height();

  GfCamera export_camera();

  CameraData camera_data;

  int screen_width;
  int screen_height;
  GfVec4i border;
};

CameraData::CameraData(Object *camera_obj, GfVec2i res, GfVec4f tile)
{
  Camera *camera = (Camera *)camera_obj->data;
  
  float t_pos[2] = {tile[0], tile[1]};
  float t_size[2] = {tile[2], tile[3]};

  copy_m4_m4(transform, camera_obj->object_to_world);

  clip_range = GfRange1f(camera->clip_start, camera->clip_end);
  mode = camera->type;

  if (camera->dof.flag & CAM_DOF_ENABLED)
  {
    float focus_distance;
    if (!camera->dof.focus_object) {
      focus_distance = camera->dof.focus_distance;
    } 
    else {
      GfVec3f obj_pos(camera->dof.focus_object->object_to_world[0][3],
                      camera->dof.focus_object->object_to_world[1][3],
                      camera->dof.focus_object->object_to_world[2][3]);
      GfVec3f cam_pos(transform[0][3],
                      transform[1][3],
                      transform[2][3]);
      focus_distance = (obj_pos - cam_pos).GetLength();
    }

    dof_data = tuple(max(focus_distance, 0.001f),
                     camera->dof.aperture_fstop,
                     camera->dof.aperture_blades);
  }

  float ratio = (float)res[0] / res[1];

  switch (camera->sensor_fit) {
    case CAMERA_SENSOR_FIT_VERT:
      lens_shift = GfVec2f(camera->shiftx / ratio, camera->shifty);
      break;
    case CAMERA_SENSOR_FIT_HOR:
      lens_shift = GfVec2f(camera->shiftx, camera->shifty * ratio);
      break;
    case CAMERA_SENSOR_FIT_AUTO:
      if (ratio > 1.0f) {
        lens_shift = GfVec2f(camera->shiftx, camera->shifty * ratio);
      }
      else {
        lens_shift = GfVec2f(camera->shiftx / ratio, camera->shifty);
      }
      break;
    default:
      lens_shift = GfVec2f(camera->shiftx, camera->shifty);
      break;
  }

  lens_shift = GfVec2f(lens_shift[0] / t_size[0] + (t_pos[0] + t_size[0] * 0.5 - 0.5) / t_size[0],
                       lens_shift[1] / t_size[1] + (t_pos[1] + t_size[1] * 0.5 - 0.5) / t_size[1]);

  switch (camera->type) {
    case CAM_PERSP:
      focal_length = camera->lens;

      switch (camera->sensor_fit) {
        case CAMERA_SENSOR_FIT_VERT:
          sensor_size = GfVec2f(camera->sensor_y * ratio, camera->sensor_y);
          break;
        case CAMERA_SENSOR_FIT_HOR:
          sensor_size = GfVec2f(camera->sensor_x, camera->sensor_x / ratio);
          break;
        case CAMERA_SENSOR_FIT_AUTO:
          if (ratio > 1.0f) {
            sensor_size = GfVec2f(camera->sensor_x, camera->sensor_x / ratio);
          }
          else {
            sensor_size = GfVec2f(camera->sensor_x * ratio, camera->sensor_x);
          }
          break;
        default:
          sensor_size = GfVec2f(camera->sensor_x, camera->sensor_y);
          break;
      }
      sensor_size = GfVec2f(sensor_size[0] * t_size[0], sensor_size[1] * t_size[1]);
      break;

    case CAM_ORTHO:
      focal_length = 0.0f;
      switch (camera->sensor_fit) {
        case CAMERA_SENSOR_FIT_VERT:
          ortho_size = GfVec2f(camera->ortho_scale * ratio, camera->ortho_scale);
          break;
        case CAMERA_SENSOR_FIT_HOR:
          ortho_size = GfVec2f(camera->ortho_scale, camera->ortho_scale / ratio);
          break;
        case CAMERA_SENSOR_FIT_AUTO:
          if (ratio > 1.0f) {
            ortho_size = GfVec2f(camera->ortho_scale, camera->ortho_scale / ratio);
          }
          else {
            ortho_size = GfVec2f(camera->ortho_scale * ratio, camera->ortho_scale);
          }
          break;
        default:
          ortho_size = GfVec2f(camera->ortho_scale, camera->ortho_scale);
          break;
      }
      ortho_size = GfVec2f(ortho_size[0] * t_size[0], ortho_size[1] * t_size[1]);
      break;

    case CAM_PANO:
      /* TODO: Recheck parameters for PANO camera */
      focal_length = camera->lens;

      switch (camera->sensor_fit) {
        case CAMERA_SENSOR_FIT_VERT:
          sensor_size = GfVec2f(camera->sensor_y * ratio, camera->sensor_y);
          break;
        case CAMERA_SENSOR_FIT_HOR:
          sensor_size = GfVec2f(camera->sensor_x, camera->sensor_x / ratio);
          break;
        case CAMERA_SENSOR_FIT_AUTO:
          if (ratio > 1.0f) {
            sensor_size = GfVec2f(camera->sensor_x, camera->sensor_x / ratio);
          }
          else {
            sensor_size = GfVec2f(camera->sensor_x * ratio, camera->sensor_x);
          }
          break;
        default:
          sensor_size = GfVec2f(camera->sensor_x, camera->sensor_y);
          break;
      }
      sensor_size = GfVec2f(sensor_size[0] * t_size[0], sensor_size[1] * t_size[1]);

    default:
      focal_length = camera->lens;
      sensor_size = GfVec2f(camera->sensor_y * ratio, camera->sensor_y);
  }
}

CameraData::CameraData(BL::Context &b_context)
{
  // this constant was found experimentally, didn't find such option in
  // context.space_data or context.region_data
  float VIEWPORT_SENSOR_SIZE = 72.0;

  BL::SpaceView3D space_data = (BL::SpaceView3D)b_context.space_data();

  GfVec2i res(b_context.region().width(), b_context.region().height());
  float ratio = (float)res[0] / res[1];

  switch (b_context.region_data().view_perspective()) {
    case BL::RegionView3D::view_perspective_PERSP: {
      mode = CAM_PERSP;
      clip_range = GfRange1f(space_data.clip_start(), space_data.clip_end());
      lens_shift = GfVec2f(0.0, 0.0);
      focal_length = space_data.lens();

      if (ratio > 1.0) {
        sensor_size = GfVec2f(VIEWPORT_SENSOR_SIZE, VIEWPORT_SENSOR_SIZE / ratio);
      }
      else {
        sensor_size = GfVec2f(VIEWPORT_SENSOR_SIZE * ratio, VIEWPORT_SENSOR_SIZE);
      }

      invert_m4_m4(transform, (float(*)[4])b_context.region_data().view_matrix().data);
      break;
    }

    case BL::RegionView3D::view_perspective_ORTHO: {
      mode = CAM_ORTHO;
      lens_shift = GfVec2f(0.0f, 0.0f);

      float o_size = b_context.region_data().view_distance() * VIEWPORT_SENSOR_SIZE / space_data.lens();
      float o_depth = space_data.clip_end();

      clip_range = GfRange1f(-o_depth * 0.5, o_depth * 0.5);

      if (ratio > 1.0f) {
        ortho_size = GfVec2f(o_size, o_size / ratio);
      } else {
        ortho_size = GfVec2f(o_size * ratio, o_size);
      }
    
      invert_m4_m4(transform, (float(*)[4])b_context.region_data().view_matrix().data);
      break;
    }

    case BL::RegionView3D::view_perspective_CAMERA: {
      BL::Object camera_obj = space_data.camera();

      *this = CameraData((Object *)camera_obj.ptr.data, res, GfVec4f(0, 0, 1, 1));

      invert_m4_m4(transform, (float(*)[4])b_context.region_data().view_matrix().data);    

      // This formula was taken from previous plugin with corresponded comment
      // See blender/intern/cycles/blender/blender_camera.cpp:blender_camera_from_view (look for 1.41421f)
      float zoom = 4.0 / pow((pow(2.0, 0.5) + b_context.region_data().view_camera_zoom() / 50.0), 2);

      // Updating l_shift due to viewport zoom and view_camera_offset
      // view_camera_offset should be multiplied by 2
      lens_shift = GfVec2f((lens_shift[0] + b_context.region_data().view_camera_offset()[0] * 2) / zoom,
                           (lens_shift[1] + b_context.region_data().view_camera_offset()[1] * 2) / zoom);

      if (mode == BL::Camera::type_ORTHO) {
        ortho_size *= zoom;
      }
      else {
        sensor_size *= zoom;
      }
      break;
    }

    default:
      break;
  }
}

GfCamera CameraData::export_gf(GfVec4f tile)
{
  float t_pos[2] = {tile[0], tile[1]}, t_size[2] = {tile[2], tile[3]};

  GfCamera gf_camera = GfCamera();

  gf_camera.SetClippingRange(clip_range);

  float l_shift[2] = {(lens_shift[0] + t_pos[0] + t_size[0] * 0.5f - 0.5f) / t_size[0],
                      (lens_shift[1] + t_pos[1] + t_size[1] * 0.5f - 0.5f) / t_size[1]};

  switch (mode) {
    case CAM_PERSP:
    case CAM_PANO: {
      /*  TODO: store panoramic camera settings */
      gf_camera.SetProjection(GfCamera::Projection::Perspective);
      gf_camera.SetFocalLength(focal_length);

      float s_size[2] = {sensor_size[0] * t_size[0], sensor_size[1] * t_size[1]};

      gf_camera.SetHorizontalAperture(s_size[0]);
      gf_camera.SetVerticalAperture(s_size[1]);

      gf_camera.SetHorizontalApertureOffset(l_shift[0] * s_size[0]);
      gf_camera.SetVerticalApertureOffset(l_shift[1] * s_size[1]);
      break;
    }
    case CAM_ORTHO: {
      gf_camera.SetProjection(GfCamera::Projection::Orthographic);

      // Use tenths of a world unit accorging to USD docs https://graphics.pixar.com/usd/docs/api/class_gf_camera.html
      float o_size[2] = {ortho_size[0] * t_size[0] * 10, 
                         ortho_size[1] * t_size[1] * 10};

      gf_camera.SetHorizontalAperture(o_size[0]);
      gf_camera.SetVerticalAperture(o_size[1]);

      gf_camera.SetHorizontalApertureOffset(l_shift[0] * o_size[0]);
      gf_camera.SetVerticalApertureOffset(l_shift[1] * o_size[1]);
      break;
    }
    default:
      break;
  }

  double transform_d[4][4];
  for (int i = 0 ; i < 4; i++) {
    for (int j = 0 ; j < 4; j++) {
      transform_d[i][j] = (double)transform[i][j];
    }
  }
  gf_camera.SetTransform(GfMatrix4d(transform_d));
  
  return gf_camera;
}

ViewSettings::ViewSettings(BL::Context &b_context)
  : camera_data(b_context)
{
  screen_width = b_context.region().width();
  screen_height = b_context.region().height();

  float width_half = screen_width / 2.0f;
  float height_half = screen_height / 2.0f;

  BL::Scene b_scene = b_context.scene();

  //getting render border
  int x1 = 0, y1 = 0;
  int x2 = screen_width, y2 = screen_height;

  if (b_context.region_data().view_perspective() == BL::RegionView3D::view_perspective_CAMERA) {
    if (b_scene.render().use_border()) {
      BL::Object b_camera_obj = b_scene.camera();
      BL::Camera b_camera = (BL::Camera)b_camera_obj.data();

      float camera_points[4][3];

      b_camera.view_frame(b_scene, camera_points[0], camera_points[1], camera_points[2], camera_points[3]);

      BL::Array<float, 16> region_persp_matrix = b_context.region_data().perspective_matrix();
      BL::Array<float, 16> camera_world_matrix = b_camera_obj.matrix_world();

      float screen_points[4][2];

      for (int i = 0 ; i < 4; i++) {
        float world_location[] = {camera_points[i][0], camera_points[i][1], camera_points[i][2], 1.0f};
        mul_m4_v4((float(*)[4])camera_world_matrix.data, world_location);
        mul_m4_v4((float(*)[4])region_persp_matrix.data, world_location);

        if (world_location[3] > 0.0) {
          screen_points[i][0] = width_half + width_half * (world_location[0] / world_location[3]);
          screen_points[i][1] = height_half + height_half * (world_location[1] / world_location[3]);
        }
      }

      // getting camera view region
      float x1_f = min({screen_points[0][0], screen_points[1][0], screen_points[2][0], screen_points[3][0]});
      float x2_f = max({screen_points[0][0], screen_points[1][0], screen_points[2][0], screen_points[3][0]});
      float y1_f = min({screen_points[0][1], screen_points[1][1], screen_points[2][1], screen_points[3][1]});
      float y2_f = max({screen_points[0][1], screen_points[1][1], screen_points[2][1], screen_points[3][1]});

      // adjusting region to border
      float x = x1_f, y = y1_f;
      float dx = x2_f - x1_f, dy = y2_f - y1_f;

      x1 = x + b_scene.render().border_min_x() * dx;
      x2 = x + b_scene.render().border_max_x() * dx;
      y1 = y + b_scene.render().border_min_y() * dy;
      y2 = y + b_scene.render().border_max_y() * dy;

      // adjusting to region screen resolution
      x1 = max(min(x1, screen_width), 0);
      x2 = max(min(x2, screen_width), 0);
      y1 = max(min(y1, screen_height), 0);
      y2 = max(min(y2, screen_height), 0);
    }
  }
  else {
    if (((BL::SpaceView3D)b_context.space_data()).use_render_border()) {
      int x = x1, y = y1;
      int dx = x2 - x1, dy = y2 - y1;

      x1 = int(x + ((BL::SpaceView3D)b_context.space_data()).render_border_min_x() * dx);
      x2 = int(x + ((BL::SpaceView3D)b_context.space_data()).render_border_max_x() * dx);
      y1 = int(y + ((BL::SpaceView3D)b_context.space_data()).render_border_min_y() * dy);
      y2 = int(y + ((BL::SpaceView3D)b_context.space_data()).render_border_max_y() * dy);
    }
  }

  border = GfVec4i(x1, y1, x2 - x1, y2 - y1);
}

int ViewSettings::width()
{
  return border[2];
}

int ViewSettings::height()
{
  return border[3];
}

GfCamera ViewSettings::export_camera()
{
  return camera_data.export_gf(GfVec4f(
    (float)border[0] / screen_width, (float)border[1] / screen_height,
    (float)border[2] / screen_width, (float)border[3] / screen_height));
}

GLTexture::GLTexture()
  : textureId(0)
  , width(0)
  , height(0)
  , channels(4)
{
}

GLTexture::~GLTexture()
{
  if (textureId) {
    free();
  }
}

void GLTexture::setBuffer(HdRenderBuffer *buffer)
{
  if (!textureId) {
    create(buffer);
    return;
  }

  if (width != buffer->GetWidth() || height != buffer->GetHeight()) {
    free();
    create(buffer);
    return;
  }

  glBindTexture(GL_TEXTURE_2D, textureId);
    
  void *data = buffer->Map();
  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA, GL_FLOAT, data);
  buffer->Unmap();
}

void GLTexture::create(HdRenderBuffer *buffer)
{
  width = buffer->GetWidth();
  height = buffer->GetHeight();
  channels = HdGetComponentCount(buffer->GetFormat());

  glGenTextures(1, &textureId);
  glBindTexture(GL_TEXTURE_2D, textureId);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  
  void *data = buffer->Map();
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, data);
  buffer->Unmap();
}

void GLTexture::free()
{
  glDeleteTextures(1, &textureId);
  textureId = 0;
}

void GLTexture::draw(GLfloat x, GLfloat y)
{
  // INITIALIZATION

  // Getting shader program
  GLint shader_program;
  glGetIntegerv(GL_CURRENT_PROGRAM, &shader_program);

  // Generate vertex array
  GLuint vertex_array;
  glGenVertexArrays(1, &vertex_array);

  GLint texturecoord_location = glGetAttribLocation(shader_program, "texCoord");
  GLint position_location = glGetAttribLocation(shader_program, "pos");

  // Generate geometry buffers for drawing textured quad
  GLfloat position[8] = { x, y, x + width, y, x + width, y + height, x, y + height };
  GLfloat texcoord[8] = {0.0, 0.0, 1.0, 0.0, 1.0, 1.0, 0.0, 1.0};

  GLuint vertex_buffer[2];
  glGenBuffers(2, vertex_buffer);
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer[0]);
  glBufferData(GL_ARRAY_BUFFER, 32, position, GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer[1]);
  glBufferData(GL_ARRAY_BUFFER, 32, texcoord, GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  // DRAWING
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, textureId);

  glBindVertexArray(vertex_array);
  glEnableVertexAttribArray(texturecoord_location);
  glEnableVertexAttribArray(position_location);

  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer[0]);
  glVertexAttribPointer(position_location, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer[1]);
  glVertexAttribPointer(texturecoord_location, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

  glBindVertexArray(0);
  glBindTexture(GL_TEXTURE_2D, 0);

  // DELETING
  glDeleteBuffers(2, vertex_buffer);
  glDeleteVertexArrays(1, &vertex_array);
}

void ViewportEngine::sync(BL::Depsgraph &b_depsgraph, BL::Context &b_context, HdRenderSettingsMap &renderSettings)
{
  if (!sceneDelegate) {
    sceneDelegate = std::make_unique<BlenderSceneDelegate>(renderIndex.get(), 
      SdfPath::AbsoluteRootPath().AppendElementString("scene"));
  }
  View3D *view3d = (View3D *)b_context.space_data().ptr.data;
  sceneDelegate->Populate(b_depsgraph, view3d);

  for (auto const& setting : renderSettings) {
    renderDelegate->SetRenderSetting(setting.first, setting.second);
  }
}

void ViewportEngine::viewDraw(BL::Depsgraph &b_depsgraph, BL::Context &b_context)
{
  ViewSettings viewSettings(b_context);
  if (viewSettings.width() * viewSettings.height() == 0) {
    return;
  };

  BL::Scene b_scene = b_depsgraph.scene_eval();
  GfCamera gfCamera = viewSettings.export_camera();

  freeCameraDelegate->SetCamera(gfCamera);
  renderTaskDelegate->SetCameraAndViewport(freeCameraDelegate->GetCameraId(), 
    GfVec4d(viewSettings.border[0], viewSettings.border[1], viewSettings.border[2], viewSettings.border[3]));

  if (!b_engine.bl_use_gpu_context()) {
    renderTaskDelegate->SetRendererAov(HdAovTokens->color);
  }
  
  HdTaskSharedPtrVector tasks = renderTaskDelegate->GetTasks();

  if (getRendererPercentDone() == 0.0f) {
    timeBegin = chrono::steady_clock::now();
  }

  b_engine.bind_display_space_shader(b_scene);

  {
    // Release the GIL before calling into hydra, in case any hydra plugins call into python.
    TF_PY_ALLOW_THREADS_IN_SCOPE();
    engine->Execute(renderIndex.get(), &tasks);

    if (!b_engine.bl_use_gpu_context()) {
      texture.setBuffer(renderTaskDelegate->GetRendererAov(HdAovTokens->color));
      texture.draw((GLfloat)viewSettings.border[0], (GLfloat)viewSettings.border[1]);
    }
  }
  
  b_engine.unbind_display_space_shader();

  chrono::time_point<chrono::steady_clock> timeCurrent = chrono::steady_clock::now();
  chrono::milliseconds elapsedTime = chrono::duration_cast<chrono::milliseconds>(timeCurrent - timeBegin);

  string formattedTime = formatDuration(elapsedTime);

  if (!renderTaskDelegate->IsConverged()) {
    notifyStatus("Time: " + formattedTime + " | Done: " + to_string(int(getRendererPercentDone())) + "%",
                 "Render");
    b_engine.tag_redraw();
  }
  else {
    notifyStatus(("Time: " + formattedTime).c_str(), "Rendering Done");
  }
}

void ViewportEngine::notifyStatus(const string &info, const string &status)
{
  b_engine.update_stats(status.c_str(), info.c_str());
}

}   // namespace blender::render::hydra
