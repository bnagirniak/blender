/* SPDX-License-Identifier: Apache-2.0
 * Copyright 2011-2022 Blender Foundation */

#include <pxr/imaging/hd/light.h>
#include <pxr/imaging/hd/material.h>
#include <pxr/usd/usdLux/tokens.h>
#include <pxr/imaging/hdSt/tokens.h>
//#include <pxr/base/vt/array.h>

#include "glog/logging.h"

#include "blenderSceneDelegate.h"
#include "object.h"

namespace blender::render::hydra {

BlenderSceneDelegate::BlenderSceneDelegate(HdRenderIndex* parentIndex, SdfPath const& delegateID, BL::Depsgraph &b_depsgraph)
  : HdSceneDelegate(parentIndex, delegateID)
  , b_depsgraph(b_depsgraph)
  , isPopulated(false)
{
}

std::unique_ptr<ObjectExport> BlenderSceneDelegate::objectExport(SdfPath const & id)
{
  //std::string name = objects[id].name;
  //for (BL::Object &obj : b_depsgraph.objects) {
  //  if (obj.name_full() == name) {
  //    return std::make_unique<ObjectExport>(obj, b_depsgraph);
  //  }
  //}
  return nullptr;
}

void BlenderSceneDelegate::updateMaterial(ObjectData &obj_data)
{
  //HdRenderIndex& index = GetRenderIndex();
  //Material *material = obj_data.material();
  //if (material) {
  //  SdfPath matId = GetDelegateID().AppendElementString(TfMakeValidIdentifier(matExport.name()));
  //  if (materials.find(matId) == materials.end()) {
  //    index.InsertSprim(HdPrimTypeTokens->material, this, matId);
  //    MaterialData matData(matExport.name());
  //    matData.mtlxPath = matExport.export_mtlx();
  //    materials[matId] = matData;
  //    LOG(INFO) << "Add material: " << matId << ", mtlx=" << matData.mtlxPath.GetResolvedPath();
  //  }
  //  objData.data["materialId"] = matId;
  //}
  //else if (objData.data.find("materialId") != objData.data.end()) {
  //  objData.data.erase("materialId");
  //}
}

ObjectData *BlenderSceneDelegate::object_data(SdfPath const &id)
{
  auto it = objects.find(id);
  if (it == objects.end()) {
    return nullptr;
  }
  return &it->second;
}

void BlenderSceneDelegate::Populate()
{
  LOG(INFO) << "Populate " << isPopulated;
  HdRenderIndex& index = GetRenderIndex();

  //if (isPopulated) {
  //  for (auto &update : b_depsgraph.updates) {
  //    BL::ID id = update.id();
  //    LOG(INFO) << "Update: " << id.name_full() << " " << update.is_updated_transform() << update.is_updated_geometry() << update.is_updated_shading();

  //    if (id.is_a(&RNA_Object)) {
  //      ObjectExport objExport((BL::Object &)id, b_depsgraph);
  //      SdfPath objId = GetDelegateID().AppendElementString(TfMakeValidIdentifier(objExport.name()));

  //      if (objects.find(objId) == objects.end()) {
  //        if (objExport.type() == BL::Object::type_MESH) {
  //          LOG(INFO) << "Add mesh object: " << objId;
  //          index.InsertRprim(HdPrimTypeTokens->mesh, this, objId);
  //          objects[objId] = __ObjectData(objExport.name(), HdPrimTypeTokens->mesh);
  //          updateMaterial(objExport);
  //        }
  //        else if (objExport.type() == BL::Object::type_LIGHT) {
  //          LOG(INFO) << "Add light object: " << objId;
  //          TfToken lightType = objExport.lightExport().type();
  //          index.InsertSprim(lightType, this, objId);
  //          objects[objId] = __ObjectData(objExport.name(), lightType);
  //        }
  //        continue;
  //      }

  //      if (update.is_updated_geometry()) {
  //        LOG(INFO) << "Full updated: " << objId;
  //        if (objExport.type() == BL::Object::type_MESH) {
  //          updateMaterial(objExport);
  //          index.GetChangeTracker().MarkRprimDirty(objId, HdChangeTracker::AllDirty);
  //        }
  //        else if (objExport.type() == BL::Object::type_LIGHT) {
  //          index.GetChangeTracker().MarkSprimDirty(objId, HdLight::AllDirty);
  //        }
  //        continue;
  //      }

  //      if (update.is_updated_transform()) {
  //        LOG(INFO) << "Transform updated: " << objId;
  //        if (objExport.type() == BL::Object::type_MESH) {
  //          index.GetChangeTracker().MarkRprimDirty(objId, HdChangeTracker::DirtyTransform);
  //        }
  //        else if (objExport.type() == BL::Object::type_LIGHT) {
  //          index.GetChangeTracker().MarkSprimDirty(objId, HdLight::DirtyTransform);
  //        }
  //      }

  //      if (update.is_updated_shading()) {
  //        LOG(INFO) << "Shading updated: " << objId;
  //        if (objExport.type() == BL::Object::type_MESH) {
  //          index.GetChangeTracker().MarkRprimDirty(objId, HdChangeTracker::DirtyMaterialId);
  //        }
  //      }
  //      continue;
  //    }

  //    if (id.is_a(&RNA_Material)) {
  //      if (update.is_updated_shading()) {
  //        MaterialExport matExport((BL::Material &)id);
  //        SdfPath matId = GetDelegateID().AppendElementString(TfMakeValidIdentifier(matExport.name()));

  //        auto it = materials.find(matId);
  //        if (it != materials.end()) {
  //          it->second.mtlxPath = matExport.export_mtlx();
  //          LOG(INFO) << "Update material: " << matId << ", mtlx=" << it->second.mtlxPath.GetResolvedPath();
  //          index.GetChangeTracker().MarkSprimDirty(matId, HdMaterial::AllDirty);
  //        }
  //      }
  //    }
  //    
  //    if (id.is_a(&RNA_Collection)) {
  //      if (update.is_updated_transform() && update.is_updated_geometry()) {
  //        /* remove unused objects */
  //        std::set<std::string> availableObjects;
  //        for (auto &inst : b_depsgraph.object_instances) {
  //          if (inst.is_instance()) {
  //            continue;
  //          }
  //          BL::Object obj = inst.object();
  //          if (obj.type() == BL::Object::type_MESH || obj.type() == BL::Object::type_LIGHT) {
  //            availableObjects.insert(obj.name_full());
  //          }
  //        }
  //        for (auto it = objects.begin(); it != objects.end(); ++it) {
  //          if (availableObjects.find(it->second.name) != availableObjects.end()) {
  //            continue;
  //          }
  //          LOG(INFO) << "Remove: " << it->first;
  //          if (index.GetRprim(it->first)) {
  //            index.RemoveRprim(it->first);
  //          }
  //          else {
  //            index.RemoveSprim(it->second.type, it->first);
  //          }
  //          objects.erase(it);
  //          it = objects.begin();
  //        }

  //        /* remove unused materials */
  //        std::set<SdfPath> availableMaterials;
  //        for (auto &obj : objects) {
  //          if (obj.second.data.find("materialId") != obj.second.data.end()) {
  //            availableMaterials.insert(obj.second.data["materialId"].Get<SdfPath>());
  //          }
  //        }
  //        for (auto it = materials.begin(); it != materials.end(); ++it) {
  //          if (availableMaterials.find(it->first) != availableMaterials.end()) {
  //            continue;
  //          }
  //          LOG(INFO) << "Remove material: " << it->first;
  //          index.RemoveSprim(HdPrimTypeTokens->material, it->first);
  //          materials.erase(it);
  //          it = materials.begin(); 
  //        }
  //      }
  //      continue;
  //    }
  //  }
  //  return;
  //}

  for (auto &inst : b_depsgraph.object_instances) {
    if (inst.is_instance()) {
      continue;
    }

    Object *object = (Object *)inst.object().ptr.data;

    if (object->type != OB_MESH && object->type != OB_LAMP) {
      continue;
    }

    ObjectData obj_data(object);
    SdfPath obj_id = GetDelegateID().AppendElementString(obj_data.path_name());
    
    if (obj_data.prim_type() == HdPrimTypeTokens->mesh) {
      LOG(INFO) << "Add mesh object: " << obj_data.name() << " id=" << obj_id;
      index.InsertRprim(obj_data.prim_type(), this, obj_id);
      objects[obj_id] = obj_data;
      updateMaterial(obj_data);
      continue;
    }
    
    if (obj_data.type() == OB_LAMP) {
      LOG(INFO) << "Add light object: " << obj_data.name() << " id=" << obj_id;
      index.InsertSprim(obj_data.prim_type(), this, obj_id);
      objects[obj_id] = obj_data;
      continue;
    }
  }
  
  isPopulated = true;
}

HdMeshTopology BlenderSceneDelegate::GetMeshTopology(SdfPath const& id)
{
  LOG(INFO) << "GetMeshTopology: " << id.GetAsString();
  ObjectData &obj_data = objects[id];
  return HdMeshTopology(PxOsdOpenSubdivTokens->catmullClark, HdTokens->rightHanded,
                        obj_data.get_data<VtIntArray>(TfToken("faceCounts")),
                        obj_data.get_data<VtIntArray>(HdTokens->pointsIndices));
}

VtValue BlenderSceneDelegate::Get(SdfPath const& id, TfToken const& key)
{
  LOG(INFO) << "Get: " << id.GetAsString() << " [" << key.GetString() << "]";
  
  VtValue ret;
  ObjectData *obj_data = object_data(id);
  if (obj_data) {
    if (obj_data->has_data(key)) {
      ret = obj_data->get_data(key);
    }
  }
  else if (key == HdStRenderBufferTokens->stormMsaaSampleCount) {
    // TODO: temporary value, it should be delivered through Python UI
    ret = 16;
  }
  else if (key.GetString() == "MaterialXFilename") {
    //MaterialData &matData = materials[id];
    //if (!matData.mtlxPath.GetResolvedPath().empty()) {
    //  ret = matData.mtlxPath;
    //}
  }
  return ret;
}

HdPrimvarDescriptorVector BlenderSceneDelegate::GetPrimvarDescriptors(SdfPath const& id, HdInterpolation interpolation)
{
  LOG(INFO) << "GetPrimvarDescriptors: " << id.GetAsString() << " " << interpolation;
  HdPrimvarDescriptorVector primvars;
  ObjectData &obj_data = objects[id];
  if (interpolation == HdInterpolationVertex) {
    if (obj_data.has_data(HdTokens->points)) {
      primvars.emplace_back(HdTokens->points, interpolation, HdPrimvarRoleTokens->point);
    }
  }
  else if (interpolation == HdInterpolationFaceVarying) {
    if (obj_data.has_data(HdTokens->normals)) {
      primvars.emplace_back(HdTokens->normals, interpolation, HdPrimvarRoleTokens->normal);
    }
    if (obj_data.has_data(HdPrimvarRoleTokens->textureCoordinate)) {
      primvars.emplace_back(HdPrimvarRoleTokens->textureCoordinate, interpolation, HdPrimvarRoleTokens->textureCoordinate);
    }
  }
  return primvars;
}

SdfPath BlenderSceneDelegate::GetMaterialId(SdfPath const & rprimId)
{
  SdfPath ret;
  //__ObjectData &objData = objects[rprimId];
  //auto it = objData.data.find("materialId");
  //if (it != objData.data.end()) {
  //  ret = it->second.Get<SdfPath>();
  //}

  //LOG(INFO) << "GetMaterialId [" << rprimId.GetAsString() << "] = " << ret.GetAsString();
  return ret;
}

VtValue BlenderSceneDelegate::GetMaterialResource(SdfPath const& materialId)
{
  LOG(INFO) << "GetMaterialResource: " << materialId.GetAsString();
  return VtValue();
}

GfMatrix4d BlenderSceneDelegate::GetTransform(SdfPath const& id)
{
  LOG(INFO) << "GetTransform: " << id.GetAsString();

  return objects[id].transform();
}

VtValue BlenderSceneDelegate::GetLightParamValue(SdfPath const& id, TfToken const& key)
{
  LOG(INFO) << "GetLightParamValue: " << id.GetAsString() << " [" << key.GetString() << "]";
  VtValue ret;
  ObjectData *obj_data = object_data(id);
  if (obj_data) {
    if (obj_data->has_data(key)) {
      ret = obj_data->get_data(key);
    }
    else if (key == HdLightTokens->exposure) {
      // TODO: temporary value, it should be delivered through Python UI
      ret = 1.0f;
    }
  }
  return ret;
}

} // namespace blender::render::hydra
