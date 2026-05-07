#include "velocity_field.h"
#include <cstring>

#ifdef _WIN32
#include <windows.h>
#include <GL/gl.h>
#else
#include <GL/gl.h>
#endif

VelocityField::VelocityField()
    : grid_size_(64), cell_size_(1.0f),
      field_texture_(0), divergence_texture_(0), curl_texture_(0),
      mapped_field_(nullptr), initialized_(false) {
}

VelocityField::~VelocityField() {
    shutdown();
}

void VelocityField::initialize(int gridSize) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (initialized_) {
        shutdown();
    }
    
    grid_size_ = gridSize;
    
    auto& pool = GPUBufferPool::instance();
    uint32_t flags = static_cast<uint32_t>(GPUBufferFlags::MAP_PERSISTENT) |
                     static_cast<uint32_t>(GPUBufferFlags::MAP_COHERENT);
    
    field_buffer_ = pool.allocateBuffer(
        grid_size_ * grid_size_ * sizeof(GPUVelocityField),
        GPUBufferUsage::STREAM,
        flags
    );
    
    mapped_field_ = static_cast<GPUVelocityField*>(field_buffer_->map());
    
    glGenTextures(1, &field_texture_);
    glBindTexture(GL_TEXTURE_2D, field_texture_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, grid_size_, grid_size_, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    glGenTextures(1, &divergence_texture_);
    glBindTexture(GL_TEXTURE_2D, divergence_texture_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, grid_size_, grid_size_, 0, GL_RED, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    glGenTextures(1, &curl_texture_);
    glBindTexture(GL_TEXTURE_2D, curl_texture_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, grid_size_, grid_size_, 0, GL_RED, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    initialized_ = true;
}

void VelocityField::shutdown() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        return;
    }
    
    if (field_buffer_) {
        field_buffer_->unmap();
        auto& pool = GPUBufferPool::instance();
        pool.releaseBuffer(field_buffer_);
        field_buffer_.reset();
    }
    
    if (field_texture_) {
        glDeleteTextures(1, &field_texture_);
        field_texture_ = 0;
    }
    
    if (divergence_texture_) {
        glDeleteTextures(1, &divergence_texture_);
        divergence_texture_ = 0;
    }
    
    if (curl_texture_) {
        glDeleteTextures(1, &curl_texture_);
        curl_texture_ = 0;
    }
    
    mapped_field_ = nullptr;
    initialized_ = false;
}

void VelocityField::updateFromAssets(const std::vector<GPUAssetInstance>& assets) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_ || !mapped_field_) {
        return;
    }
    
    memset(mapped_field_, 0, grid_size_ * grid_size_ * sizeof(GPUVelocityField));
    
    for (const auto& asset : assets) {
        int gx = static_cast<int>(asset.x / cell_size_);
        int gy = static_cast<int>(asset.y / cell_size_);
        
        if (gx >= 0 && gx < grid_size_ && gy >= 0 && gy < grid_size_) {
            int idx = gy * grid_size_ + gx;
            mapped_field_[idx].vx += asset.velocity_x;
            mapped_field_[idx].vy += asset.velocity_y;
            mapped_field_[idx].density += 1.0f;
        }
    }
    
    for (int i = 0; i < grid_size_ * grid_size_; ++i) {
        if (mapped_field_[i].density > 0.0f) {
            mapped_field_[i].vx /= mapped_field_[i].density;
            mapped_field_[i].vy /= mapped_field_[i].density;
        }
    }
    
    updateTexture();
}

void VelocityField::updateTexture() {
    if (!field_texture_ || !mapped_field_) {
        return;
    }
    
    glBindTexture(GL_TEXTURE_2D, field_texture_);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, grid_size_, grid_size_, 
                    GL_RGBA, GL_FLOAT, mapped_field_);
}

void VelocityField::computeDivergence() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_ || !mapped_field_) {
        return;
    }
    
    std::vector<float> divergence(grid_size_ * grid_size_, 0.0f);
    
    for (int y = 1; y < grid_size_ - 1; ++y) {
        for (int x = 1; x < grid_size_ - 1; ++x) {
            int idx = y * grid_size_ + x;
            
            float vxRight = mapped_field_[(y) * grid_size_ + (x + 1)].vx;
            float vxLeft = mapped_field_[(y) * grid_size_ + (x - 1)].vx;
            float vyUp = mapped_field_[(y + 1) * grid_size_ + (x)].vy;
            float vyDown = mapped_field_[(y - 1) * grid_size_ + (x)].vy;
            
            divergence[idx] = (vxRight - vxLeft + vyUp - vyDown) / (2.0f * cell_size_);
        }
    }
    
    glBindTexture(GL_TEXTURE_2D, divergence_texture_);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, grid_size_, grid_size_, 
                    GL_RED, GL_FLOAT, divergence.data());
}

void VelocityField::computeCurl() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_ || !mapped_field_) {
        return;
    }
    
    std::vector<float> curl(grid_size_ * grid_size_, 0.0f);
    
    for (int y = 1; y < grid_size_ - 1; ++y) {
        for (int x = 1; x < grid_size_ - 1; ++x) {
            int idx = y * grid_size_ + x;
            
            float vxUp = mapped_field_[(y + 1) * grid_size_ + (x)].vx;
            float vxDown = mapped_field_[(y - 1) * grid_size_ + (x)].vx;
            float vyRight = mapped_field_[(y) * grid_size_ + (x + 1)].vy;
            float vyLeft = mapped_field_[(y) * grid_size_ + (x - 1)].vy;
            
            curl[idx] = (vyRight - vyLeft - vxUp + vxDown) / (2.0f * cell_size_);
        }
    }
    
    glBindTexture(GL_TEXTURE_2D, curl_texture_);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, grid_size_, grid_size_, 
                    GL_RED, GL_FLOAT, curl.data());
}

uint32_t VelocityField::getFieldTextureId() const {
    return field_texture_;
}

uint32_t VelocityField::getDivergenceTextureId() const {
    return divergence_texture_;
}

uint32_t VelocityField::getCurlTextureId() const {
    return curl_texture_;
}

void VelocityField::getVelocityAt(float x, float y, float& vx, float& vy) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_ || !mapped_field_) {
        vx = 0.0f;
        vy = 0.0f;
        return;
    }
    
    int gx = static_cast<int>(x / cell_size_);
    int gy = static_cast<int>(y / cell_size_);
    
    if (gx >= 0 && gx < grid_size_ && gy >= 0 && gy < grid_size_) {
        int idx = gy * grid_size_ + gx;
        vx = mapped_field_[idx].vx;
        vy = mapped_field_[idx].vy;
    } else {
        vx = 0.0f;
        vy = 0.0f;
    }
}

float VelocityField::getDensityAt(float x, float y) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_ || !mapped_field_) {
        return 0.0f;
    }
    
    int gx = static_cast<int>(x / cell_size_);
    int gy = static_cast<int>(y / cell_size_);
    
    if (gx >= 0 && gx < grid_size_ && gy >= 0 && gy < grid_size_) {
        return mapped_field_[gy * grid_size_ + gx].density;
    }
    
    return 0.0f;
}

int VelocityField::getGridSize() const {
    return grid_size_;
}

float VelocityField::getCellSize() const {
    return cell_size_;
}