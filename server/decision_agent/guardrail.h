#pragma once
#include <string>
#include "../models/ranked_action.h"

namespace v33 {

struct RiskScore {
    double missing_risk;
    double inactivity_risk;
    double abnormal_risk;
};

enum class GuardrailTrigger {
    NONE,
    HIGH_RISK_MISSING,
    HIGH_RISK_INACTIVITY,
    ILLEGAL_LOCATION,
    HIGH_UNCERTAINTY,
    LOW_CONFIDENCE
};

class Guardrail {
public:
    static bool shouldForceInspect(const RiskScore& risk);
    static bool shouldForceAlert(bool in_illegal_location);

    static GuardrailTrigger checkTrigger(const RiskScore& risk, bool in_illegal_location);

    static std::string getReason(GuardrailTrigger trigger);
    static std::string getReason();

private:
    static constexpr double HIGH_RISK_THRESHOLD = 0.9;
    static constexpr double HIGH_UNCERTAINTY_THRESHOLD = 0.4;
    static constexpr double LOW_CONFIDENCE_THRESHOLD = 0.6;
};

}
