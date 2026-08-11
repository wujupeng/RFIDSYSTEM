#include "source_hash_calculator.h"
#include <algorithm>

namespace pa::spatial_truth {

static uint64_t fnv1a(const std::string& s) {
    uint64_t hash = 14695981039346656037ULL;
    for (char c : s) {
        hash ^= static_cast<uint64_t>(c);
        hash *= 1099511628211ULL;
    }
    return hash;
}

uint64_t SourceHashCalculator::calculate(const std::vector<ObservationData>& observations) const {
    auto sorted = observations;
    std::sort(sorted.begin(), sorted.end(), [](const ObservationData& a, const ObservationData& b) {
        if (a.reader_id != b.reader_id) return a.reader_id < b.reader_id;
        return a.timestamp < b.timestamp;
    });

    std::string buf;
    for (const auto& obs : sorted) {
        buf += obs.epc + "|" + obs.reader_id + "|" +
               std::to_string(obs.rssi) + "|" + std::to_string(obs.phase) + "|" +
               std::to_string(obs.timestamp) + ";";
    }
    return fnv1a(buf);
}

uint64_t SourceHashCalculator::calculateEnvironmentHash(const RFEnvironmentFeatures& rf) const {
    std::string buf = std::to_string(rf.channel_quality) + "|" +
                      std::to_string(rf.interference_level) + "|" +
                      std::to_string(rf.multipath_factor);
    return fnv1a(buf);
}

} // namespace pa::spatial_truth