/* SPDX-License-Identifier: Apache-2.0
 * Copyright 2011-2022 Blender Foundation */

#pragma once

#include <map>

#include <pxr/pxr.h>
#include <pxr/imaging/hd/camera.h>
#include <pxr/imaging/hd/sceneDelegate.h>
#include <pxr/imaging/hd/renderIndex.h>
#include <pxr/usd/usd/stage.h>

#include "MEM_guardedalloc.h"
#include "RNA_blender_cpp.h"

#include "object.h"

using namespace pxr;

namespace blender::render::hydra {

class BlenderSceneDelegate : public HdSceneDelegate {
public:
  BlenderSceneDelegate(HdRenderIndex* renderIndex, SdfPath const &delegateId, BL::Depsgraph &b_depsgraph);
  ~BlenderSceneDelegate() override = default;

  void Populate();

  // delegate methods
  HdMeshTopology GetMeshTopology(SdfPath const& id) override;
  GfMatrix4d GetTransform(SdfPath const& id) override;
  VtValue Get(SdfPath const& id, TfToken const& key) override;
  VtValue GetLightParamValue(SdfPath const& id, TfToken const& key) override;
  HdPrimvarDescriptorVector GetPrimvarDescriptors(SdfPath const& id, HdInterpolation interpolation) override;
  SdfPath GetMaterialId(SdfPath const &rprimId) override;
  VtValue GetMaterialResource(SdfPath const &materialId) override;

private:
  ObjectData *object_data(SdfPath const &id);
  MaterialData *material_data(SdfPath const &id);
  SdfPath object_id(Object *object);
  SdfPath material_id(Material *material);
  bool supported_object(Object *object);

  void add_update_object(Object *object, bool geometry, bool transform, bool shading);
  void set_material(ObjectData &obj_data);
  void update_material(Material *material);
  void update_collection();

  BL::Depsgraph b_depsgraph;
  bool is_populated;
  ObjectDataMap objects;
  MaterialDataMap materials;
};

} // namespace blender::render::hydra
