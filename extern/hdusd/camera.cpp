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
  // TODO: add code
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
    data.mode = BL::Camera::type_PERSP;
    data.clip_range = {((BL::SpaceView3D)b_context.space_data()).clip_start(), ((BL::SpaceView3D)b_context.space_data()).clip_end()};
    data.lens_shift = {0.0, 0.0};
    data.focal_length = ((BL::SpaceView3D)b_context.space_data()).lens();

    if (ratio > 1.0) {
      data.sensor_size = {VIEWPORT_SENSOR_SIZE, VIEWPORT_SENSOR_SIZE / ratio};
    }
    else {
      data.sensor_size = {VIEWPORT_SENSOR_SIZE * ratio, VIEWPORT_SENSOR_SIZE};
    }
    vector<vector<float>> vect = hdusd::matrix::convert_array_4x4_to_vector(b_context.region_data().view_matrix());
    data.transform = hdusd::matrix::convert_vector_to_array_4x4(hdusd::matrix::get_inverse(vect));
  }
  else if (b_context.region_data().view_perspective() == BL::RegionView3D::view_perspective_ORTHO) {
    // TODO: add code
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

    if (data.mode == BL::Camera::type_ORTHO) {
      data.ortho_size = {data.ortho_size[0] * zoom, data.ortho_size[1] * zoom};
    }
    else {
      data.sensor_size = {data.sensor_size[0] * zoom, data.sensor_size[1] * zoom};
    }
  }

  return data;
}

pxr::GfCamera CameraData::export_gf(vector<float> tile)
{
  vector<float> tile_pos = {tile[0], tile[1]}, tile_size = {tile[2], tile[3]};

  pxr::GfCamera gf_camera = pxr::GfCamera();

  gf_camera.SetClippingRange(pxr::GfRange1f(this->clip_range[0], this->clip_range[1]));

  vector<float> lens_shift = {(float)(this->lens_shift[0] + tile_pos[0] + tile_size[0] * 0.5 - 0.5) / tile_size[0],
                              (float)(this->lens_shift[1] + tile_pos[1] + tile_size[1] * 0.5 - 0.5) / tile_size[1]};

  if (this->mode == BL::Camera::type_PERSP) {
    gf_camera.SetProjection(pxr::GfCamera::Projection::Perspective);
    gf_camera.SetFocalLength(this->focal_length);

    vector<float> sensor_size = {this->sensor_size[0] * tile_size[0], this->sensor_size[1] * tile_size[1]};

    gf_camera.SetHorizontalAperture(sensor_size[0]);
    gf_camera.SetVerticalAperture(sensor_size[1]);

    gf_camera.SetHorizontalApertureOffset(lens_shift[0] * sensor_size[0]);
    gf_camera.SetVerticalApertureOffset(lens_shift[1] * sensor_size[1]);
  }
  else if (this->mode == BL::Camera::type_ORTHO) {
    // TODO: add code
  }
  else if (this->mode == BL::Camera::type_PANO) {
    // TODO: store panoramic camera settings
  }

  double double_array[(sizeof(this->transform.data) / sizeof(this->transform.data[0]))];

  for (int i = 0 ; i < (sizeof(this->transform.data) / sizeof(this->transform.data[0])); i++) {
    double_array[i] = (double)this->transform.data[i];
  }

  gf_camera.SetTransform(pxr::GfMatrix4d(double_array[0], double_array[1], double_array[2], double_array[3],
                                         double_array[4], double_array[5], double_array[6], double_array[7],
                                         double_array[8], double_array[9], double_array[10], double_array[11],
                                         double_array[12], double_array[13], double_array[14], double_array[15])
                        );
  
  return gf_camera;
}
