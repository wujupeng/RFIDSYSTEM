#include "spatial_filter.h"
#include <cmath>
#include <algorithm>

SpatialFilter::SpatialFilter() {}

double SpatialFilter::gaussian(double x, double sigma) {
    return exp(-x * x / (2 * sigma * sigma));
}

TagPosition SpatialFilter::applyMovingAverage(const std::vector<TagPosition>& positions) {
    TagPosition pos;
    pos.confidence = 0.0;
    
    if (positions.empty()) {
        return pos;
    }
    
    double sum_x = 0.0, sum_y = 0.0;
    double sum_conf = 0.0;
    
    for (const auto& p : positions) {
        sum_x += p.x * p.confidence;
        sum_y += p.y * p.confidence;
        sum_conf += p.confidence;
    }
    
    if (sum_conf > 0) {
        pos.x = sum_x / sum_conf;
        pos.y = sum_y / sum_conf;
        pos.confidence = sum_conf / positions.size();
    }
    
    return pos;
}

TagPosition SpatialFilter::applyMedianFilter(const std::vector<TagPosition>& positions) {
    TagPosition pos;
    pos.confidence = 0.0;
    
    if (positions.empty()) {
        return pos;
    }
    
    std::vector<double> xs, ys;
    for (const auto& p : positions) {
        xs.push_back(p.x);
        ys.push_back(p.y);
    }
    
    std::sort(xs.begin(), xs.end());
    std::sort(ys.begin(), ys.end());
    
    size_t n = xs.size();
    if (n % 2 == 0) {
        pos.x = (xs[n/2 - 1] + xs[n/2]) / 2;
        pos.y = (ys[n/2 - 1] + ys[n/2]) / 2;
    } else {
        pos.x = xs[n/2];
        pos.y = ys[n/2];
    }
    
    pos.confidence = 0.85;
    return pos;
}

TagPosition SpatialFilter::applyGaussianFilter(const std::vector<TagPosition>& positions) {
    TagPosition pos;
    pos.confidence = 0.0;
    
    if (positions.empty()) {
        return pos;
    }
    
    double sum_x = 0.0, sum_y = 0.0;
    double sum_weight = 0.0;
    
    int n = positions.size();
    for (int i = 0; i < n; ++i) {
        double dist = std::abs(i - n / 2);
        double weight = gaussian(dist, n / 3.0);
        
        sum_x += positions[i].x * weight;
        sum_y += positions[i].y * weight;
        sum_weight += weight;
    }
    
    if (sum_weight > 0) {
        pos.x = sum_x / sum_weight;
        pos.y = sum_y / sum_weight;
        pos.confidence = 0.9;
    }
    
    return pos;
}

bool SpatialFilter::isValidPosition(const TagPosition& pos,
                                   const TagPosition& last_pos,
                                   double max_speed) {
    double dx = pos.x - last_pos.x;
    double dy = pos.y - last_pos.y;
    double distance = sqrt(dx * dx + dy * dy);
    
    return distance < max_speed;
}