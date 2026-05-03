#pragma once
#include <string>
#include <vector>
#include <optional>

namespace data {

class EPCValidator {
public:
    static bool isValidEPC(const std::string& epc);
    
    static std::string normalizeEPC(const std::string& epc);
    
    static bool isHex(const std::string& str);
    
    static bool hasValidLength(const std::string& epc);
    
    static std::optional<std::string> validateAndNormalize(const std::string& epc);
    
    static std::vector<std::string> filterValidEPCs(const std::vector<std::string>& epcs);
    
    static std::vector<std::string> filterInvalidEPCs(const std::vector<std::string>& epcs);
};

} // namespace data