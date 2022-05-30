/* SPDX-License-Identifier: Apache-2.0
 * Copyright 2011-2022 Blender Foundation */

#include "session.h"

namespace hdusd {

std::unique_ptr<pxr::UsdStageCache> stageCache;
std::unique_ptr<pxr::UsdImagingGLEngine> imagingGLEngine;

BlenderSession::BlenderSession(BL::RenderEngine &b_engine, BL::BlendData &b_data)
    : b_engine(b_engine)
    , b_data(b_data)
{
}

BlenderSession::~BlenderSession()
{
}

}   // namespace hdusd
