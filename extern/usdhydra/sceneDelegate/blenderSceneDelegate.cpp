/* SPDX-License-Identifier: Apache-2.0
 * Copyright 2011-2022 Blender Foundation */

#include <pxr/imaging/hd/light.h>
#include <pxr/imaging/hd/material.h>
#include <pxr/usd/usdLux/tokens.h>

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
  std::string name = std::get<0>(objects[id]);
  for (BL::Object &obj : b_depsgraph.objects) {
    if (obj.name_full() == name) {
      return std::make_unique<ObjectExport>(obj, b_depsgraph);
    }
  }
  return nullptr;
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
        BL::Object &obj = (BL::Object &)id;
        std::string objName = obj.name_full();
        SdfPath objId = GetDelegateID().AppendElementString(TfMakeValidIdentifier(objName));
        
        if (objects.find(objId) == objects.end()) {
          if (obj.type() == BL::Object::type_MESH) {
            LOG(INFO) << "Add mesh object: " << objId;
            index.InsertRprim(HdPrimTypeTokens->mesh, this, objId);
            objects[objId] = std::make_tuple(objName, TfToken(0));
          }
          else if (obj.type() == BL::Object::type_LIGHT) {
            LOG(INFO) << "Add light object: " << objId;
            TfToken lightType = ObjectExport(obj, b_depsgraph).lightExport().type();
            index.InsertSprim(lightType, this, objId);
            objects[objId] = std::make_tuple(objName, lightType);
          }
          continue;
        }
        if (update.is_updated_geometry()) {
          LOG(INFO) << "Full updated: " << objId;
          if (obj.type() == BL::Object::type_MESH) {
            index.GetChangeTracker().MarkRprimDirty(objId, HdChangeTracker::AllDirty);
          }
          else if (obj.type() == BL::Object::type_LIGHT) {
            index.GetChangeTracker().MarkSprimDirty(objId, HdChangeTracker::AllDirty);
          }
          continue;
        }
        if (update.is_updated_transform()) {
          LOG(INFO) << "Transform updated: " << objId;
          if (obj.type() == BL::Object::type_MESH) {
            index.GetChangeTracker().MarkRprimDirty(objId, HdChangeTracker::DirtyTransform);
          }
          else if (obj.type() == BL::Object::type_LIGHT) {
            index.GetChangeTracker().MarkSprimDirty(objId, HdLight::DirtyTransform);
          }
        }
        continue;
      }
      
      if (id.is_a(&RNA_Collection)) {
        BL::Collection &col = (BL::Collection &)id;
        if (update.is_updated_transform() && update.is_updated_geometry()) {
          //available objects from depsgraph
          std::set<std::string> depsObjects;
          for (auto &inst : b_depsgraph.object_instances) {
            if (inst.is_instance()) {
              continue;
            }
            BL::Object obj = inst.object();
            if (obj.type() == BL::Object::type_MESH || obj.type() == BL::Object::type_LIGHT) {
              depsObjects.insert(obj.name_full());
            }
          }
          
          auto it = objects.begin();
          while (it != objects.end()) {
            if (depsObjects.find(std::get<0>(it->second)) == depsObjects.end()) {
              LOG(INFO) << "Removed: " << it->first;
              if (index.GetRprim(it->first)) {
                index.RemoveRprim(it->first);
              }
              else {
                index.RemoveSprim(std::get<1>(it->second), it->first);
              }
              objects.erase(it);
              it = objects.begin();
            }
            else {
              ++it;
            }
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
    
    BL::Object obj = inst.object();
    SdfPath objId = GetDelegateID().AppendElementString(TfMakeValidIdentifier(obj.name_full()));
    
    if (obj.type() == BL::Object::type_MESH) {
      LOG(INFO) << "Add mesh object: " << objId;
      index.InsertRprim(HdPrimTypeTokens->mesh, this, objId);
      objects[objId] = std::make_tuple(obj.name_full(), TfToken(0));
      //index.GetChangeTracker().MarkRprimDirty(objId, HdChangeTracker::DirtyMaterialId);

      continue;
    }
    if (obj.type() == BL::Object::type_LIGHT) {
      LOG(INFO) << "Add light object: " << objId;
      TfToken lightType = ObjectExport(obj, b_depsgraph).lightExport().type();
      index.InsertSprim(lightType, this, objId);
      objects[objId] = std::make_tuple(obj.name_full(), lightType);
      continue;
    }
  }

  index.InsertSprim(HdPrimTypeTokens->material, this, GetDelegateID().AppendElementString("Material"));
  //index.GetChangeTracker().MarkSprimDirty(GetDelegateID().AppendElementString("Material"), HdMaterial::DirtyResource);
  
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
  if (key == HdTokens->points) {
    VtVec3fArray points = objectExport(id)->meshExport().vertices();
    return VtValue(points);
  }
  if (key == HdTokens->normals) {
    VtVec3fArray normals = objectExport(id)->meshExport().normals();
    return VtValue(normals);
  }
  if (key.GetString() == "MaterialXFilename") {
    return VtValue(SdfAssetPath("D:\\amd\\usd\\a\\Material.mtlx"));
  }
  return VtValue();
}

HdPrimvarDescriptorVector BlenderSceneDelegate::GetPrimvarDescriptors(SdfPath const& id, HdInterpolation interpolation)
{
  LOG(INFO) << "GetPrimvarDescriptors: " << id.GetAsString() << " " << interpolation;
  HdPrimvarDescriptorVector primvars;
  if (interpolation == HdInterpolationVertex) {
    primvars.emplace_back(HdTokens->points, interpolation, HdPrimvarRoleTokens->point);
  }
  if (interpolation == HdInterpolationFaceVarying) {
    primvars.emplace_back(HdTokens->normals, interpolation, HdPrimvarRoleTokens->normal);
  }
  return primvars;
}

SdfPath BlenderSceneDelegate::GetMaterialId(SdfPath const & rprimId)
{
  SdfPath ret;
  if (rprimId == GetDelegateID().AppendElementString("Cube")) {
    ret = SdfPath::AbsoluteRootPath().AppendElementString("materials").AppendElementString("Material").AppendElementString("Materials").AppendElementString("surfacematerial_2");
  }
  else {
    ret = GetDelegateID().AppendElementString("Material");
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

  return VtValue();
}

} // namespace usdhydra
