#include "guardrail.h"

namespace v33 {

bool Guardrail::shouldForceInspect(const RiskScore& risk) {
    return risk.missing_risk > HIGH_RISK_THRESHOLD ||
           risk.inactivity_risk > HIGH_RISK_THRESHOLD;
}

bool Guardrail::shouldForceAlert(bool in_illegal_location) {
    return in_illegal_location;
}

GuardrailTrigger Guardrail::checkTrigger(const RiskScore& risk, bool in_illegal_location) {
    if (in_illegal_location) {
        return GuardrailTrigger::ILLEGAL_LOCATION;
    }

    if (risk.missing_risk > HIGH_RISK_THRESHOLD) {
        return GuardrailTrigger::HIGH_RISK_MISSING;
    }

    if (risk.inactivity_risk > HIGH_RISK_THRESHOLD) {
        return GuardrailTrigger::HIGH_RISK_INACTIVITY;
    }

    return GuardrailTrigger::NONE;
}

std::string Guardrail::getReason(GuardrailTrigger trigger) {
    switch (trigger) {
        case GuardrailTrigger::ILLEGAL_LOCATION:
            return "ILLEGAL_LOCATION_GUARDRAIL";
        case GuardrailTrigger::HIGH_RISK_MISSING:
            return "HIGH_RISK_MISSING_GUARDRAIL";
        case GuardrailTrigger::HIGH_RISK_INACTIVITY:
            return "HIGH_RISK_INACTIVITY_GUARDRAIL";
        case GuardrailTrigger::HIGH_UNCERTAINTY:
            return "HIGH_UNCERTAINTY_FALLBACK";
        case GuardrailTrigger::LOW_CONFIDENCE:
            return "LOW_CONFIDENCE_SHOW_TOPK";
        default:
            return "Triggered by Guardrail";
    }
}

std::string Guardrail::getReason() {
    return "Triggered by Guardrail";
}

}
