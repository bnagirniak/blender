/* SPDX-License-Identifier: Apache-2.0
 * Copyright 2011-2022 Blender Foundation */

#include <pxr/imaging/hd/light.h>
#include <pxr/imaging/hd/material.h>
#include <pxr/usd/usdLux/tokens.h>
#include <pxr/imaging/hdSt/tokens.h>

#include "glog/logging.h"

#include "blenderSceneDelegate.h"
#include "object.h"

namespace usdhydra {

BlenderSceneDelegate::BlenderSceneDelegate(HdRenderIndex* parentIndex, SdfPath const& delegateID, BL::Depsgraph &b_depsgraph)
  : HdSceneDelegate(parentIndex, delegateID)
  , b_depsgraph(b_depsgraph)
  , isPopulated(false)
{
}

std::unique_ptr<ObjectExport> BlenderSceneDelegate::objectExport(SdfPath const & id)
{
  std::string name = objects[id].name;
  for (BL::Object &obj : b_depsgraph.objects) {
    if (obj.name_full() == name) {
      return std::make_unique<ObjectExport>(obj, b_depsgraph);
    }
  }
  return nullptr;
}

void BlenderSceneDelegate::updateMaterial(ObjectExport & objExport)
{
  HdRenderIndex& index = GetRenderIndex();
  SdfPath objId = GetDelegateID().AppendElementString(TfMakeValidIdentifier(objExport.name()));
  ObjectData &objData = objects[objId];
  MaterialExport matExport = objExport.materialExport();
  if (matExport) {
    SdfPath matId = GetDelegateID().AppendElementString(TfMakeValidIdentifier(matExport.name()));
    if (materials.find(matId) == materials.end()) {
      index.InsertSprim(HdPrimTypeTokens->material, this, matId);
      MaterialData matData(matExport.name());
      matData.mtlxPath = matExport.exportMX();
      materials[matId] = matData;
      LOG(INFO) << "Add material: " << matId << ", mtlx=" << matData.mtlxPath.GetResolvedPath();
    }
    objData.data["materialId"] = matId;
  }
  else if (objData.data.find("materialId") != objData.data.end()) {
    objData.data.erase("materialId");
  }
}

void BlenderSceneDelegate::Populate()
{
  LOG(INFO) << "Populate " << isPopulated;
  HdRenderIndex& index = GetRenderIndex();

  if (isPopulated) {
    for (auto &update : b_depsgraph.updates) {
      BL::ID id = update.id();
      LOG(INFO) << "Update: " << id.name_full() << " " << update.is_updated_transform() << update.is_updated_geometry() << update.is_updated_shading();

      if (id.is_a(&RNA_Object)) {
        ObjectExport objExport((BL::Object &)id, b_depsgraph);
        SdfPath objId = GetDelegateID().AppendElementString(TfMakeValidIdentifier(objExport.name()));

        if (objects.find(objId) == objects.end()) {
          if (objExport.type() == BL::Object::type_MESH) {
            LOG(INFO) << "Add mesh object: " << objId;
            index.InsertRprim(HdPrimTypeTokens->mesh, this, objId);
            objects[objId] = ObjectData(objExport.name(), HdPrimTypeTokens->mesh);
            updateMaterial(objExport);
          }
          else if (objExport.type() == BL::Object::type_LIGHT) {
            LOG(INFO) << "Add light object: " << objId;
            TfToken lightType = objExport.lightExport().type();
            index.InsertSprim(lightType, this, objId);
            objects[objId] = ObjectData(objExport.name(), lightType);
          }
          continue;
        }

        if (update.is_updated_geometry()) {
          LOG(INFO) << "Full updated: " << objId;
          if (objExport.type() == BL::Object::type_MESH) {
            updateMaterial(objExport);
            index.GetChangeTracker().MarkRprimDirty(objId, HdChangeTracker::AllDirty);
          }
          else if (objExport.type() == BL::Object::type_LIGHT) {
            index.GetChangeTracker().MarkSprimDirty(objId, HdLight::AllDirty);
          }
          continue;
        }

        if (update.is_updated_transform()) {
          LOG(INFO) << "Transform updated: " << objId;
          if (objExport.type() == BL::Object::type_MESH) {
            index.GetChangeTracker().MarkRprimDirty(objId, HdChangeTracker::DirtyTransform);
          }
          else if (objExport.type() == BL::Object::type_LIGHT) {
            index.GetChangeTracker().MarkSprimDirty(objId, HdLight::DirtyTransform);
          }
        }

        if (update.is_updated_shading()) {
          LOG(INFO) << "Shading updated: " << objId;
          if (objExport.type() == BL::Object::type_MESH) {
            index.GetChangeTracker().MarkRprimDirty(objId, HdChangeTracker::DirtyMaterialId);
          }
        }
        continue;
      }

      if (id.is_a(&RNA_Material)) {
        if (update.is_updated_shading()) {
          MaterialExport matExport((BL::Material &)id);
          SdfPath matId = GetDelegateID().AppendElementString(TfMakeValidIdentifier(matExport.name()));

          auto it = materials.find(matId);
          if (it == materials.end()) {

          }
          else {
            it->second.mtlxPath = matExport.exportMX();
            LOG(INFO) << "Update material: " << matId << ", mtlx=" << it->second.mtlxPath.GetResolvedPath();
            index.GetChangeTracker().MarkSprimDirty(matId, HdMaterial::AllDirty);
          }
        }
      }
      
      if (id.is_a(&RNA_Collection)) {
        BL::Collection &col = (BL::Collection &)id;
        if (update.is_updated_transform() && update.is_updated_geometry()) {
          // remove unused objects
          std::set<std::string> availableObjects;
          for (auto &inst : b_depsgraph.object_instances) {
            if (inst.is_instance()) {
              continue;
            }
            BL::Object obj = inst.object();
            if (obj.type() == BL::Object::type_MESH || obj.type() == BL::Object::type_LIGHT) {
              availableObjects.insert(obj.name_full());
            }
          }
          for (auto it = objects.begin(); it != objects.end(); ++it) {
            if (availableObjects.find(it->second.name) != availableObjects.end()) {
              continue;
            }
            LOG(INFO) << "Remove: " << it->first;
            if (index.GetRprim(it->first)) {
              index.RemoveRprim(it->first);
            }
            else {
              index.RemoveSprim(it->second.type, it->first);
            }
            objects.erase(it);
            it = objects.begin();
          }

          // remove unused materials
          std::set<SdfPath> availableMaterials;
          for (auto &obj : objects) {
            if (obj.second.data.find("materialId") != obj.second.data.end()) {
              availableMaterials.insert(obj.second.data["materialId"].Get<SdfPath>());
            }
          }
          for (auto it = materials.begin(); it != materials.end(); ++it) {
            if (availableMaterials.find(it->first) != availableMaterials.end()) {
              continue;
            }
            LOG(INFO) << "Remove material: " << it->first;
            index.RemoveSprim(HdPrimTypeTokens->material, it->first);
            materials.erase(it);
            it = materials.begin(); 
          }
        }
        continue;
      }
    }
    return;
  }

  for (auto &inst : b_depsgraph.object_instances) {
    if (inst.is_instance()) {
      continue;
    }
    
    ObjectExport objExport(inst.object(), b_depsgraph);
    SdfPath objId = GetDelegateID().AppendElementString(TfMakeValidIdentifier(objExport.name()));
    
    if (objExport.type() == BL::Object::type_MESH) {
      LOG(INFO) << "Add mesh object: " << objId;
      index.InsertRprim(HdPrimTypeTokens->mesh, this, objId);
      objects[objId] = ObjectData(objExport.name(), HdPrimTypeTokens->mesh);
      updateMaterial(objExport);
      continue;
    }
    
    if (objExport.type() == BL::Object::type_LIGHT) {
      LOG(INFO) << "Add light object: " << objId;
      TfToken lightType = objExport.lightExport().type();
      index.InsertSprim(lightType, this, objId);
      objects[objId] = ObjectData(objExport.name(), lightType);
      continue;
    }
  }
  
  isPopulated = true;
}

HdMeshTopology BlenderSceneDelegate::GetMeshTopology(SdfPath const& id)
{
  LOG(INFO) << "GetMeshTopology: " << id.GetAsString();
  MeshExport meshExport = objectExport(id)->meshExport();
  return HdMeshTopology(PxOsdOpenSubdivTokens->catmullClark, HdTokens->rightHanded,
                        meshExport.faceVertexCounts(), meshExport.faceVertexIndices());
}

VtValue BlenderSceneDelegate::Get(SdfPath const& id, TfToken const& key)
{
  LOG(INFO) << "Get: " << id.GetAsString() << " [" << key.GetString() << "]";
  
  VtValue ret;
  if (key == HdPrimvarRoleTokens->point) {
    ret = objectExport(id)->meshExport().vertices();
  }
  else if (key == HdPrimvarRoleTokens->normal) {
    ret = objectExport(id)->meshExport().normals();
  }
  else if (key == HdPrimvarRoleTokens->textureCoordinate) {
    ret = objectExport(id)->meshExport().uvs();
  }
  else if (key == HdStRenderBufferTokens->stormMsaaSampleCount) {
    // TODO: temporary value, it should be delivered through Python UI
    ret = 16;
  }
  else if (key.GetString() == "MaterialXFilename") {
    MaterialData &matData = materials[id];
    if (!matData.mtlxPath.GetResolvedPath().empty()) {
      ret = matData.mtlxPath;
    }
  }
  return ret;
}

HdPrimvarDescriptorVector BlenderSceneDelegate::GetPrimvarDescriptors(SdfPath const& id, HdInterpolation interpolation)
{
  LOG(INFO) << "GetPrimvarDescriptors: " << id.GetAsString() << " " << interpolation;
  HdPrimvarDescriptorVector primvars;
  if (interpolation == HdInterpolationVertex) {
    primvars.emplace_back(HdPrimvarRoleTokens->point, interpolation, HdPrimvarRoleTokens->point);
  }
  if (interpolation == HdInterpolationFaceVarying) {
    primvars.emplace_back(HdPrimvarRoleTokens->normal, interpolation, HdPrimvarRoleTokens->normal);
    primvars.emplace_back(HdPrimvarRoleTokens->textureCoordinate, interpolation, HdPrimvarRoleTokens->textureCoordinate);
  }
  return primvars;
}

SdfPath BlenderSceneDelegate::GetMaterialId(SdfPath const & rprimId)
{
  SdfPath ret;
  ObjectData &objData = objects[rprimId];
  auto it = objData.data.find("materialId");
  if (it != objData.data.end()) {
    ret = it->second.Get<SdfPath>();
  }

  LOG(INFO) << "GetMaterialId [" << rprimId.GetAsString() << "] = " << ret.GetAsString();
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
  return objectExport(id)->transform();
}

VtValue BlenderSceneDelegate::GetLightParamValue(SdfPath const& id, TfToken const& key)
{
  LOG(INFO) << "GetLightParamValue: " << id.GetAsString() << " [" << key.GetString() << "]";

  if (key == HdLightTokens->intensity) {
    return objectExport(id)->lightExport().intensity();
  }
  if (key == HdLightTokens->width) {
    return objectExport(id)->lightExport().width();
  }
  if (key == HdLightTokens->height) {
    return objectExport(id)->lightExport().height();
  }
  if (key == HdLightTokens->radius) {
    return objectExport(id)->lightExport().radius();
  }
  if (key == HdLightTokens->color) {
    return objectExport(id)->lightExport().color();
  }  
  if (key == HdLightTokens->angle) {
    return objectExport(id)->lightExport().angle();
  }
  if (key == HdLightTokens->shapingConeAngle) {
    return objectExport(id)->lightExport().shapingConeAngle();
  }
  if (key == HdLightTokens->shapingConeSoftness) {
    return objectExport(id)->lightExport().shapingConeSoftness();
  }
  if (key == UsdLuxTokens->treatAsPoint) {
    return objectExport(id)->lightExport().treatAsPoint();
  }
  if (key == HdLightTokens->exposure) {
    // TODO: temporary value, it should be delivered through Python UI
    return VtValue(1.0f);
  } 
  return VtValue();
}

} // namespace usdhydra
