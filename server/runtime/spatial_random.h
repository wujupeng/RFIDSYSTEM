#pragma once

#include <cstdint>
#include <atomic>
#include <mutex>

class SpatialRandom {
public:
    static SpatialRandom& instance();
    
    void setSeed(uint64_t seed);
    uint64_t getSeed() const;
    
    void reset();
    
    uint32_t nextUInt();
    uint64_t nextUInt64();
    
    float nextFloat();
    float nextFloat(float min, float max);
    
    double nextDouble();
    double nextDouble(double min, double max);
    
    int32_t nextInt(int32_t min, int32_t max);
    uint32_t nextUInt(uint32_t min, uint32_t max);
    
    bool nextBool();
    
private:
    SpatialRandom();
    
    uint64_t next();
    
    std::mutex mutex_;
    uint64_t state_;
    uint64_t seed_;
    
    static constexpr uint64_t DEFAULT_SEED = 0xdeadbeefdeadbeefULL;
    static constexpr uint64_t MULTIPLIER = 6364136223846793005ULL;
    static constexpr uint64_t INCREMENT = 1442695040888963407ULL;
};