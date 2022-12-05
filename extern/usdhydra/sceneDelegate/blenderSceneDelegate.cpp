/* SPDX-License-Identifier: Apache-2.0
 * Copyright 2011-2022 Blender Foundation */

#include <iostream>
#include "blenderSceneDelegate.h"

namespace usdhydra {

BlenderSceneDelegate::BlenderSceneDelegate(HdRenderIndex* parentIndex, SdfPath const& delegateID, BL::Depsgraph &b_depsgraph)
  : HdSceneDelegate(parentIndex, delegateID)
  , _depsgraph(b_depsgraph)
  , _isPopulated(false)
{
}

void BlenderSceneDelegate::Sync(HdSyncRequestVector* request)
{
  std::cout << "Sync " << _isPopulated << "\n";

  if (_isPopulated) {
    return;
  }

  auto &instances = _depsgraph.object_instances;
  for (auto& inst : instances) {
    if (inst.is_instance()) {
      continue;
    }
    
    BL::Object obj = inst.object();
    SdfPath objId = GetDelegateID().AppendElementString(obj.name_full());
    
    if (obj.type() == BL::Object::type_MESH) {
      GetRenderIndex().InsertRprim(HdPrimTypeTokens->mesh, this, objId);
      //GetRenderIndex().GetChangeTracker().MarkRprimDirty(objId, 
      //  HdChangeTracker::DirtyPoints | HdChangeTracker::DirtyNormals | HdChangeTracker::AllDirty);
      continue;
    }
    if (obj.type() == BL::Object::type_LIGHT) {
      GetRenderIndex().InsertSprim(HdPrimTypeTokens->sphereLight, this, objId);
      continue;
    }
    if (obj.type() == BL::Object::type_CAMERA) {
      GetRenderIndex().InsertSprim(HdPrimTypeTokens->camera, this, objId);
      continue;
    }

  }
  _isPopulated = true;
}

HdMeshTopology BlenderSceneDelegate::GetMeshTopology(SdfPath const& id)
{
  std::cout << "GetMeshTopology: " << id.GetAsString() << "\n";

  VtIntArray faceVertexCounts = {3};
  VtIntArray faceVertexIndices = {0, 1, 2};

  return HdMeshTopology(PxOsdOpenSubdivTokens->catmullClark, HdTokens->rightHanded,
                        faceVertexCounts, faceVertexIndices);
}

VtValue BlenderSceneDelegate::Get(SdfPath const& id, TfToken const& key)
{
  std::cout << "Get: " << id.GetAsString() << " [" << key.GetString() << "]\n";

  // tasks
  _ValueCache *vcache = TfMapLookupPtr(_valueCacheMap, id);
  VtValue ret;
  if (vcache && TfMapLookup(*vcache, key, &ret)) {
      return ret;
  }

  //// prims
  //if (key == HdTokens->points) {
  //    if(_meshes.find(id) != _meshes.end()) {
  //        return VtValue(_meshes[id].points);
  //    }
  //} else if (key == HdTokens->displayColor) {
  //    if(_meshes.find(id) != _meshes.end()) {
  //        return VtValue(_meshes[id].color);
  //    }
  //} else if (key == HdTokens->displayOpacity) {
  //    if(_meshes.find(id) != _meshes.end()) {
  //        return VtValue(_meshes[id].opacity);
  //    }
  //} else if (key == HdInstancerTokens->scale) {
  //    if (_instancers.find(id) != _instancers.end()) {
  //        return VtValue(_instancers[id].scale);
  //    }
  //} else if (key == HdInstancerTokens->rotate) {
  //    if (_instancers.find(id) != _instancers.end()) {
  //        return VtValue(_instancers[id].rotate);
  //    }
  //} else if (key == HdInstancerTokens->translate) {
  //    if (_instancers.find(id) != _instancers.end()) {
  //        return VtValue(_instancers[id].translate);
  //    }
  //}
  return VtValue();
}

HdPrimvarDescriptorVector BlenderSceneDelegate::GetPrimvarDescriptors(SdfPath const& id, HdInterpolation interpolation)
{
  std::cout << "GetPrimvarDescriptors: " << id.GetAsString() << " " << interpolation << "\n";
  HdPrimvarDescriptorVector primvars;
  if (interpolation == HdInterpolationVertex) {
    primvars.emplace_back(HdTokens->points, interpolation, HdPrimvarRoleTokens->point);
    primvars.emplace_back(HdTokens->normals, interpolation, HdPrimvarRoleTokens->normal);
  } 
  return primvars;
}

GfMatrix4d BlenderSceneDelegate::GetTransform(SdfPath const& id)
{
  std::cout << "GetTransform: " << id.GetAsString() << " " << id.GetName() <<"\n";
  BL::Object object = _depsgraph.objects[id.GetName()];
  if (!object) {
    return GfMatrix4d(1);
  }

  auto m = object.matrix_world();
  return GfMatrix4d(
    m[0], m[1], m[2], m[3],
    m[4], m[5], m[6], m[7],
    m[8], m[9], m[10], m[11],
    m[12], m[13], m[14], m[15]);
}

VtValue BlenderSceneDelegate::GetCameraParamValue(SdfPath const& id, TfToken const& key)
{
  std::cout << "GetCameraParamValue: " << id.GetAsString() << " [" << key.GetString() << "]\n";
  return VtValue();
}

VtValue BlenderSceneDelegate::GetLightParamValue(SdfPath const& id, TfToken const& key)
{
  std::cout << "GetLightParamValue: " << id.GetAsString() << " [" << key.GetString() << "]\n";
  return VtValue();
}

} // namespace usdhydra
