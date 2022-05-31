/* SPDX-License-Identifier: Apache-2.0
 * Copyright 2011-2022 Blender Foundation */

#include "camera.h"
#include "utils.h"

CameraData::CameraData() {
}

CameraData::~CameraData()
{
}

CameraData CameraData::init_from_camera(BL::Camera b_camera, BL::Array<float,16> b_transform, float ratio, vector<vector<int>> border)
{
  return CameraData();
}

CameraData CameraData::init_from_context(BL::Context b_context)
{
  // this constant was found experimentally, didn't find such option in
  // context.space_data or context.region_data
  float VIEWPORT_SENSOR_SIZE = 72.0;

  CameraData data;
  float ratio = (float)b_context.region().width() / (float)b_context.region().height();
  if (b_context.region_data().view_perspective() == BL::RegionView3D::view_perspective_PERSP) {  
    data = CameraData();
    data.mode = "PERSP";
    data.clip_plane = {((BL::SpaceView3D)b_context.space_data()).clip_start(), ((BL::SpaceView3D)b_context.space_data()).clip_end()};
    data.lens_shift = {0.0, 0.0};
    data.focal_length = ((BL::SpaceView3D)b_context.space_data()).lens();

    if (ratio > 1.0) {
      data.sensor_size = {VIEWPORT_SENSOR_SIZE, VIEWPORT_SENSOR_SIZE / ratio};
    }
    else {
      data.sensor_size = {VIEWPORT_SENSOR_SIZE * ratio, VIEWPORT_SENSOR_SIZE};
    }
    vector<vector<float>> vect = hdusd::convert_array_4x4_to_vector(b_context.region_data().view_matrix());
    data.transform = hdusd::convert_vector_to_array_4x4(hdusd::getInverse(vect));
  }
  else if (b_context.region_data().view_perspective() == BL::RegionView3D::view_perspective_ORTHO) {
    data = CameraData();
  }
  else if (b_context.region_data().view_perspective() == BL::RegionView3D::view_perspective_CAMERA) {
    BL::Object camera_obj = ((BL::SpaceView3D)b_context.space_data()).camera();

    data = CameraData::init_from_camera((BL::Camera)camera_obj.data(), b_context.region_data().view_matrix(), ratio);

    // This formula was taken from previous plugin with corresponded comment
    // See blender/intern/cycles/blender/blender_camera.cpp:blender_camera_from_view (look for 1.41421f)
    //float zoom = 4.0 / pow((pow(2.0, 0.5) + b_context.region_data().view_camera_zoom() / 50.0), 2);
    float zoom = 2.0 / pow((pow(2.0, 0.5) + b_context.region_data().view_camera_zoom() / 50.0), 2);

    // Updating lens_shift due to viewport zoom and view_camera_offset
    // view_camera_offset should be multiplied by 2
    data.lens_shift = {(data.lens_shift[0] + b_context.region_data().view_camera_offset()[0] * 2) / zoom,
                        (data.lens_shift[1] + b_context.region_data().view_camera_offset()[1] * 2) / zoom};

    if (data.mode == "ORTHO") {
      data.ortho_size = {data.ortho_size[0] * zoom, data.ortho_size[1] * zoom};
    }
    else {
      data.sensor_size = {data.sensor_size[0] * zoom, data.sensor_size[1] * zoom};
    }
  }

  return data;
}

pxr::GfCamera CameraData::export_gf(vector<float> tile) {
  vector<float> tile_pos = {tile[0], tile[1]}, tile_size = {tile[2], tile[3]};

  pxr::GfCamera gf_camera = pxr::GfCamera();

  gf_camera.SetClippingRange(pxr::GfRange1f(this->clip_plane[0], this->clip_plane[1]));

  vector<float> lens_shift = {(float)(this->lens_shift[0] + tile_pos[0] + tile_size[0] * 0.5 - 0.5) / tile_size[0],
                              (float)(this->lens_shift[1] + tile_pos[1] + tile_size[1] * 0.5 - 0.5) / tile_size[1]};

  if (this->mode == "PERSP") {
    gf_camera.SetProjection(pxr::GfCamera::Projection::Perspective);
    gf_camera.SetFocalLength(this->focal_length);

    vector<float> sensor_size = {this->sensor_size[0] * tile_size[0], this->sensor_size[1] * tile_size[1]};

    gf_camera.SetHorizontalAperture(sensor_size[0]);
    gf_camera.SetVerticalAperture(sensor_size[1]);

    gf_camera.SetHorizontalApertureOffset(lens_shift[0] * sensor_size[0]);
    gf_camera.SetVerticalApertureOffset(lens_shift[1] * sensor_size[1]);
  }
  else if (this->mode == "ORTHO") {

  }
  else if (this->mode == "PANO") {
  }

  double doubleArray[(sizeof(this->transform.data) / sizeof(this->transform.data[0]))];

  for (int i = 0 ; i < (sizeof(this->transform.data) / sizeof(this->transform.data[0])); i++) {
    doubleArray[i] = (double)this->transform.data[i];
  }

  gf_camera.SetTransform(pxr::GfMatrix4d(doubleArray[0], doubleArray[1], doubleArray[2], doubleArray[3],
                                         doubleArray[4], doubleArray[5], doubleArray[6], doubleArray[7],
                                         doubleArray[8], doubleArray[9], doubleArray[10], doubleArray[11],
                                         doubleArray[12], doubleArray[13], doubleArray[14], doubleArray[15])
                        );

  /*gf_camera.SetTransform(pxr::GfMatrix4d(doubleArray[0], doubleArray[4], doubleArray[8], doubleArray[12],
                                         doubleArray[1], doubleArray[5], doubleArray[9], doubleArray[13],
                                         doubleArray[2], doubleArray[6], doubleArray[10], doubleArray[14],
                                         doubleArray[3], doubleArray[7], doubleArray[11], doubleArray[15])
                        );*/
  
  return gf_camera;
}