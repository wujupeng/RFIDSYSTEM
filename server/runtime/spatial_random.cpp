#include "spatial_random.h"

SpatialRandom::SpatialRandom()
    : state_(DEFAULT_SEED), seed_(DEFAULT_SEED) {
}

SpatialRandom& SpatialRandom::instance() {
    static SpatialRandom instance;
    return instance;
}

void SpatialRandom::setSeed(uint64_t seed) {
    std::lock_guard<std::mutex> lock(mutex_);
    seed_ = seed;
    state_ = seed;
}

uint64_t SpatialRandom::getSeed() const {
    return seed_;
}

void SpatialRandom::reset() {
    std::lock_guard<std::mutex> lock(mutex_);
    state_ = seed_;
}

uint64_t SpatialRandom::next() {
    state_ = state_ * MULTIPLIER + INCREMENT;
    return state_;
}

uint32_t SpatialRandom::nextUInt() {
    std::lock_guard<std::mutex> lock(mutex_);
    return static_cast<uint32_t>(next());
}

uint64_t SpatialRandom::nextUInt64() {
    std::lock_guard<std::mutex> lock(mutex_);
    return next();
}

float SpatialRandom::nextFloat() {
    std::lock_guard<std::mutex> lock(mutex_);
    uint64_t x = next();
    return static_cast<float>(x >> 12) * (1.0f / 4294967296.0f);
}

float SpatialRandom::nextFloat(float min, float max) {
    return min + nextFloat() * (max - min);
}

double SpatialRandom::nextDouble() {
    std::lock_guard<std::mutex> lock(mutex_);
    uint64_t x = next();
    return static_cast<double>(x) * (1.0 / 18446744073709551616.0);
}

double SpatialRandom::nextDouble(double min, double max) {
    return min + nextDouble() * (max - min);
}

int32_t SpatialRandom::nextInt(int32_t min, int32_t max) {
    if (min >= max) {
        return min;
    }
    return static_cast<int32_t>(min + nextDouble() * (max - min));
}

uint32_t SpatialRandom::nextUInt(uint32_t min, uint32_t max) {
    if (min >= max) {
        return min;
    }
    return static_cast<uint32_t>(min + nextDouble() * (max - min));
}

bool SpatialRandom::nextBool() {
    std::lock_guard<std::mutex> lock(mutex_);
    return (next() & 1) == 1;
}