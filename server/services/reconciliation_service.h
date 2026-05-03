#pragma once
#include <string>
#include <vector>

namespace data {

struct ReconciliationResult {
    int total_expected;
    int total_found;
    int total_missing;
    int total_extra;
    
    double accuracy_rate;
    double profit_rate;
    double loss_rate;
    
    std::vector<std::string> missing_epcs;
    std::vector<std::string> extra_epcs;
    std::vector<std::string> found_epcs;
};

struct DirtyDataRecord {
    int id;
    std::string raw_epc;
    std::string reason;
    std::string source_reader;
    int64_t timestamp;
};

class ReconciliationService {
public:
    static ReconciliationService& instance();
    
    ReconciliationResult reconcile(int taskId, const std::vector<std::string>& scannedEPCs);
    
    void recordDirtyData(const std::string& rawEPC, const std::string& reason, const std::string& sourceReader = "");
    
    std::vector<DirtyDataRecord> getDirtyData(int limit = 100);
    
    int getDirtyDataCount();
    
    void clearDirtyData();
    
private:
    ReconciliationService();
    ~ReconciliationService() = default;
    ReconciliationService(const ReconciliationService&) = delete;
    ReconciliationService& operator=(const ReconciliationService&) = delete;
};

} // namespace data