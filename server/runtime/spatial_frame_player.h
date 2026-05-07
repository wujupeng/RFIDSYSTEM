#pragma once

#include <fstream>
#include <string>
#include <mutex>
#include <atomic>
#include <queue>
#include <condition_variable>
#include <thread>

class SpatialFrame;

class SpatialFramePlayer {
public:
    enum class PlaybackState {
        STOPPED,
        PLAYING,
        PAUSED
    };
    
    SpatialFramePlayer();
    ~SpatialFramePlayer();
    
    bool open(const std::string& path);
    void close();
    
    bool isOpen() const;
    
    // Playback control
    bool seek(uint64_t timestamp);
    bool seekToFrame(uint64_t frameId);
    
    bool nextFrame(SpatialFrame& frame);
    bool readFrameAt(uint64_t index, SpatialFrame& frame);
    
    void setSpeed(double speed);
    double getSpeed() const;
    
    void play();
    void pause();
    void stop();
    
    PlaybackState getState() const;
    
    // Metadata
    uint64_t totalFrames() const;
    uint64_t firstTimestamp() const;
    uint64_t lastTimestamp() const;
    uint64_t currentFrameId() const;
    uint64_t currentTimestamp() const;
    
    // For deterministic verification
    uint64_t getLastFrameHash() const;
    
private:
    void loadIndex();
    bool readHeader(std::ifstream& file, FrameRecordHeader& header);
    bool verifyChecksum(const FrameRecordHeader& header, const uint8_t* data);
    
    void playbackLoop();
    
    std::ifstream file_;
    std::mutex mutex_;
    
    // Index for fast seeking
    std::vector<std::streampos> framePositions_;
    std::vector<uint64_t> frameTimestamps_;
    
    // Playback state
    std::atomic<PlaybackState> state_;
    std::atomic<double> speed_;
    std::atomic<uint64_t> currentFrameIndex_;
    std::atomic<uint64_t> lastFrameHash_;
    
    // For continuous playback
    std::thread playbackThread_;
    std::queue<SpatialFrame> frameQueue_;
    std::condition_variable queueCondition_;
    std::atomic<bool> running_;
    
    static constexpr size_t MAX_QUEUE_SIZE = 3; // Ring buffer: latest 3 frames
};