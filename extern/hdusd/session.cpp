/* SPDX-License-Identifier: Apache-2.0
 * Copyright 2011-2022 Blender Foundation */

#include "session.h"

namespace hdusd {

std::unique_ptr<pxr::UsdStageCache> stageCache;

BlenderSession::BlenderSession(BL::RenderEngine &b_engine)
    : b_engine(b_engine)
{
  imagingGLEngine = std::make_unique<pxr::UsdImagingGLEngine>();
}

BlenderSession::~BlenderSession()
{
}

}   // namespace hdusd
