#pragma once

#include "../spatial_frame.h"
#include "../ai_analysis_context.h"

struct KernelTickContext {
    SpatialFrame frame;
    AIAnalysisContext ai_context;

    bool replay_mode = false;
    bool force_safe_policy = false;
};