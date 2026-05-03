#include "epc_validator.h"
#include <cctype>
#include <algorithm>
#include <optional>

namespace data {

bool EPCValidator::isValidEPC(const std::string& epc) {
    if (epc.empty()) {
        return false;
    }
    
    std::string normalized = normalizeEPC(epc);
    
    if (!hasValidLength(normalized)) {
        return false;
    }
    
    if (!isHex(normalized)) {
        return false;
    }
    
    return true;
}

std::string EPCValidator::normalizeEPC(const std::string& epc) {
    std::string result;
    result.reserve(epc.size());
    
    for (char c : epc) {
        if (c != ' ' && c != '-' && c != ':') {
            result += std::toupper(c);
        }
    }
    
    return result;
}

bool EPCValidator::isHex(const std::string& str) {
    return std::all_of(str.begin(), str.end(), [](unsigned char c) {
        return std::isxdigit(c);
    });
}

bool EPCValidator::hasValidLength(const std::string& epc) {
    size_t len = epc.length();
    
    return len == 24 || len == 16 || len == 12 || len == 8;
}

std::optional<std::string> EPCValidator::validateAndNormalize(const std::string& epc) {
    std::string normalized = normalizeEPC(epc);
    
    if (isValidEPC(normalized)) {
        return normalized;
    }
    
    return std::nullopt;
}

std::vector<std::string> EPCValidator::filterValidEPCs(const std::vector<std::string>& epcs) {
    std::vector<std::string> valid;
    valid.reserve(epcs.size());
    
    for (const auto& epc : epcs) {
        auto normalized = validateAndNormalize(epc);
        if (normalized) {
            valid.push_back(*normalized);
        }
    }
    
    return valid;
}

std::vector<std::string> EPCValidator::filterInvalidEPCs(const std::vector<std::string>& epcs) {
    std::vector<std::string> invalid;
    invalid.reserve(epcs.size());
    
    for (const auto& epc : epcs) {
        if (!isValidEPC(normalizeEPC(epc))) {
            invalid.push_back(epc);
        }
    }
    
    return invalid;
}

} // namespace data