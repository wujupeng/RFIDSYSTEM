#include "rssi_localizer.h"
#include <cmath>

RSSILocalizer::RSSILocalizer() {}

double RSSILocalizer::estimateDistance(double rssi, double tx_power, double n) {
    if (rssi >= tx_power) {
        return 0.0;
    }
    return pow(10.0, (tx_power - rssi) / (10.0 * n));
}

double RSSILocalizer::calculateDistance(double rssi) {
    return estimateDistance(rssi);
}

SpatialProbability RSSILocalizer::localize(const std::vector<TagObservation>& observations,
                                           const std::vector<ReaderInfo>& readers) {
    SpatialProbability prob;
    
    if (observations.empty()) {
        return prob;
    }
    
    double sum_x = 0.0, sum_y = 0.0;
    double sum_weight = 0.0;
    
    for (const auto& obs : observations) {
        auto it = std::find_if(readers.begin(), readers.end(),
            [&](const ReaderInfo& r) { return r.reader_id == obs.reader_id; });
        
        if (it != readers.end()) {
            double distance = estimateDistance(obs.rssi);
            double weight = std::max(0.1, 1.0 / (distance * distance + 0.1));
            
            sum_x += it->x * weight;
            sum_y += it->y * weight;
            sum_weight += weight;
        }
    }
    
    if (sum_weight > 0) {
        prob.mean_x = sum_x / sum_weight;
        prob.mean_y = sum_y / sum_weight;
        prob.covariance_xx = 9.0;
        prob.covariance_yy = 9.0;
    }
    
    return prob;
}