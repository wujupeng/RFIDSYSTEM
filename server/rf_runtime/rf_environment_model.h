#pragma once

#include <cstdint>
#include <string>
#include <map>

struct RFPhenomenon {
    std::string type;
    double intensity;
    double timestamp;
};

class RFEnvironmentModel {
public:
    static RFEnvironmentModel& instance();
    
    void addPhenomenon(const RFPhenomenon& phenomenon);
    
    void updateAttenuation(double x, double y, double value);
    
    double getAttenuation(double x, double y);
    
    void detectInterference();
    
    double getNoiseFloor();
    
    uint64_t getEnvironmentHash();
    
private:
    RFEnvironmentModel();
    
    std::vector<RFPhenomenon> phenomena_;
    std::map<std::string, double> attenuation_map_;
};