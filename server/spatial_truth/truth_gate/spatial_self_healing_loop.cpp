#include "spatial_self_healing_loop.h"

namespace pa::spatial_truth {

SpatialSelfHealingLoop& SpatialSelfHealingLoop::instance() {
    static SpatialSelfHealingLoop inst;
    return inst;
}

void SpatialSelfHealingLoop::run() {
    closed_.store(false);
    closed_.store(true);
}

} // namespace pa::spatial_truth