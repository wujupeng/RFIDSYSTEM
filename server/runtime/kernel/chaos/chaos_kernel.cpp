#include "chaos_kernel.h"
#include "../microkernel.h"
#include "../spatial_random.h"
#include "chaos_policy_engine.h"
#include "fault_injector.h"

ChaosKernel& ChaosKernel::instance() {
    static ChaosKernel k;
    return k;
}

void ChaosKernel::tick(const KernelTickContext& ctx) {
    if (!chaos_enabled_) return;

    double trust = MicroKernel::instance().getTrustScore();

    ChaosPolicy policy = ChaosPolicyEngine::sample(trust);

    auto& random = SpatialRandom::instance();
    double r = random.nextDouble();

    if (r < policy.probability) {
        FaultInjector::inject(policy.type, policy.severity);
    }
}

void ChaosKernel::enableChaos(bool enable) {
    chaos_enabled_ = enable;
}

bool ChaosKernel::isChaosEnabled() const {
    return chaos_enabled_;
}