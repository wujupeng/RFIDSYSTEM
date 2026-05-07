#pragma once

#include <fstream>
#include <string>
#include <mutex>
#include <atomic>

class SpatialFrame;

struct FrameRecordHeader {
    static constexpr uint64_t MAGIC = 0x5350415449414C46ULL; // "SPATIALF"
    
    uint64_t magic;
    uint64_t timestamp;
    uint32_t payload_size;
    uint32_t checksum;
};

class SpatialFrameRecorder {
public:
    SpatialFrameRecorder();
    ~SpatialFrameRecorder();
    
    bool open(const std::string& path);
    void close();
    
    bool isOpen() const;
    
    void append(const SpatialFrame& frame);
    void flush();
    
    uint64_t totalFrames() const;
    uint64_t totalBytes() const;
    
private:
    uint32_t computeChecksum(const uint8_t* data, size_t size);
    
    std::ofstream file_;
    std::mutex mutex_;
    
    std::atomic<uint64_t> total_frames_;
    std::atomic<uint64_t> total_bytes_;
    
    bool opened_;
};