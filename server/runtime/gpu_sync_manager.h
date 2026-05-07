#pragma once

#include <QObject>
#include <QMutex>
#include <QSharedPointer>
#include <atomic>

class SpatialFrame;

struct GPUFrameData {
    qint64 frameId;
    double timestamp;
    float* assetData;
    float* heatmapData;
    float* trajectoryData;
    int assetCount;
    int heatmapSize;
    int trajectoryCount;
};

class GPUSyncManager : public QObject {
    Q_OBJECT
    
public:
    GPUSyncManager(QObject* parent = nullptr);
    ~GPUSyncManager();
    
    void initialize();
    
    void syncFrame(const SpatialFrame& frame);
    
    QSharedPointer<GPUFrameData> getCurrentFrameData() const;
    
    void setSyncInterval(int ms);
    int syncInterval() const;
    
    void setGPUMemoryLimit(size_t bytes);
    size_t gpuMemoryLimit() const;
    
    void flush();
    
signals:
    void frameSynced(qint64 frameId);
    void gpuMemoryWarning(size_t used, size_t limit);
    
private:
    void allocateGPUBuffers();
    void releaseGPUBuffers();
    void copyToGPU(const SpatialFrame& frame);
    
    QMutex mutex_;
    int syncInterval_;
    size_t gpuMemoryLimit_;
    size_t currentGPUMemoryUsage_;
    
    QSharedPointer<GPUFrameData> currentFrame_;
    QSharedPointer<GPUFrameData> nextFrame_;
    
    std::atomic<bool> syncInProgress_;
    
    // Double buffering
    bool frontBuffer_;
    float* buffer1_;
    float* buffer2_;
    size_t bufferSize_;
};