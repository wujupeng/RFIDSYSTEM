#include "gpu_sync_manager.h"
#include "spatial_frame.h"

GPUSyncManager::GPUSyncManager(QObject* parent) 
    : QObject(parent), syncInterval_(16), 
      gpuMemoryLimit_(512 * 1024 * 1024), // 512MB
      currentGPUMemoryUsage_(0),
      syncInProgress_(false),
      frontBuffer_(true),
      buffer1_(nullptr),
      buffer2_(nullptr),
      bufferSize_(0) {
}

GPUSyncManager::~GPUSyncManager() {
    releaseGPUBuffers();
}

void GPUSyncManager::initialize() {
    allocateGPUBuffers();
}

void GPUSyncManager::syncFrame(const SpatialFrame& frame) {
    if (syncInProgress_.exchange(true)) {
        return; // Skip if sync is already in progress
    }
    
    QMutexLocker locker(&mutex_);
    
    try {
        copyToGPU(frame);
        std::swap(currentFrame_, nextFrame_);
        emit frameSynced(frame.frameId);
    } catch (...) {
        // Handle GPU sync error
    }
    
    syncInProgress_.store(false);
}

QSharedPointer<GPUFrameData> GPUSyncManager::getCurrentFrameData() const {
    QMutexLocker locker(const_cast<QMutex*>(&mutex_));
    return currentFrame_;
}

void GPUSyncManager::setSyncInterval(int ms) {
    syncInterval_ = ms;
}

int GPUSyncManager::syncInterval() const {
    return syncInterval_;
}

void GPUSyncManager::setGPUMemoryLimit(size_t bytes) {
    gpuMemoryLimit_ = bytes;
}

size_t GPUSyncManager::gpuMemoryLimit() const {
    return gpuMemoryLimit_;
}

void GPUSyncManager::flush() {
    QMutexLocker locker(&mutex_);
    currentFrame_.reset();
    nextFrame_.reset();
}

void GPUSyncManager::allocateGPUBuffers() {
    releaseGPUBuffers();
    
    bufferSize_ = 1024 * 1024 * 4; // 4MB per buffer
    buffer1_ = new float[bufferSize_];
    buffer2_ = new float[bufferSize_];
    
    currentFrame_ = QSharedPointer<GPUFrameData>::create();
    nextFrame_ = QSharedPointer<GPUFrameData>::create();
}

void GPUSyncManager::releaseGPUBuffers() {
    delete[] buffer1_;
    delete[] buffer2_;
    buffer1_ = nullptr;
    buffer2_ = nullptr;
    bufferSize_ = 0;
}

void GPUSyncManager::copyToGPU(const SpatialFrame& frame) {
    Q_UNUSED(frame);
    // In a real implementation, this would copy data to GPU memory
    // using CUDA, Vulkan, or OpenGL buffer operations
}