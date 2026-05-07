#include "instanced_asset_renderer.h"
#include "../gpu/gpu_frame_uploader.h"
#include <cstring>

#ifdef _WIN32
#include <windows.h>
#include <GL/gl.h>
#else
#include <GL/gl.h>
#endif

const char* vertexShaderSource = R"(
#version 330 core

layout(location = 0) in vec2 quadPos;
layout(location = 1) in vec2 instancePos;
layout(location = 2) in float velocityX;
layout(location = 3) in float velocityY;
layout(location = 4) in float risk;
layout(location = 5) in vec4 color;
layout(location = 6) in float size;
layout(location = 7) in float heat;
layout(location = 8) in uint flags;

uniform mat4 projection;
uniform float pointSize;
uniform float time;

out vec4 vColor;
out float vRisk;
out float vHeat;

void main() {
    vec2 pos = quadPos * size + instancePos;
    gl_Position = projection * vec4(pos, 0.0, 1.0);
    
    vColor = color;
    vRisk = risk;
    vHeat = heat;
}
)";

const char* fragmentShaderSource = R"(
#version 330 core

in vec4 vColor;
in float vRisk;
in float vHeat;

out vec4 fragColor;

void main() {
    vec2 center = gl_PointCoord - vec2(0.5);
    float dist = length(center);
    
    if (dist > 0.5) {
        discard;
    }
    
    float alpha = 1.0 - smoothstep(0.3, 0.5, dist);
    
    vec3 color = vColor.rgb;
    
    if (vRisk > 0.8) {
        float pulse = 0.8 + 0.2 * sin(gl_FragCoord.x * 0.01 + gl_FragCoord.y * 0.01);
        color *= pulse;
    }
    
    fragColor = vec4(color, alpha * vColor.a);
}
)";

InstancedAssetRenderer::InstancedAssetRenderer()
    : shader_program_(0), vao_(0), vbo_(0),
      point_size_(8.0f), glow_enabled_(true),
      initialized_(false) {
    
    memset(projection_, 0, sizeof(projection_));
    projection_[0] = 1.0f;
    projection_[5] = 1.0f;
    projection_[10] = 1.0f;
    projection_[15] = 1.0f;
}

InstancedAssetRenderer::~InstancedAssetRenderer() {
    shutdown();
}

void InstancedAssetRenderer::initialize() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (initialized_) {
        return;
    }
    
    createShaders();
    createGeometry();
    setupVertexAttributes();
    
    initialized_ = true;
}

void InstancedAssetRenderer::shutdown() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        return;
    }
    
    if (vao_) {
        glDeleteVertexArrays(1, &vao_);
        vao_ = 0;
    }
    
    if (vbo_) {
        glDeleteBuffers(1, &vbo_);
        vbo_ = 0;
    }
    
    if (shader_program_) {
        glDeleteProgram(shader_program_);
        shader_program_ = 0;
    }
    
    initialized_ = false;
}

void InstancedAssetRenderer::createShaders() {
    uint32_t vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);
    
    uint32_t fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);
    
    shader_program_ = glCreateProgram();
    glAttachShader(shader_program_, vertexShader);
    glAttachShader(shader_program_, fragmentShader);
    glLinkProgram(shader_program_);
    
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

void InstancedAssetRenderer::createGeometry() {
    float quadVertices[] = {
        -0.5f, -0.5f,
         0.5f, -0.5f,
        -0.5f,  0.5f,
         0.5f,  0.5f
    };
    
    glGenVertexArrays(1, &vao_);
    glBindVertexArray(vao_);
    
    glGenBuffers(1, &vbo_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    
    glBindVertexArray(0);
}

void InstancedAssetRenderer::setupVertexAttributes() {
    glBindVertexArray(vao_);
    
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    
    glBindVertexArray(0);
}

void InstancedAssetRenderer::render(const GPUFrameUploader& uploader,
                                   float viewX, float viewY,
                                   float viewWidth, float viewHeight) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        return;
    }
    
    uint32_t assetBufferId = uploader.getAssetBufferId();
    uint32_t assetCount = uploader.getAssetCount();
    
    if (assetBufferId == 0 || assetCount == 0) {
        return;
    }
    
    glUseProgram(shader_program_);
    
    int projLoc = glGetUniformLocation(shader_program_, "projection");
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, projection_);
    
    int sizeLoc = glGetUniformLocation(shader_program_, "pointSize");
    glUniform1f(sizeLoc, point_size_);
    
    glBindVertexArray(vao_);
    
    glBindBuffer(GL_ARRAY_BUFFER, assetBufferId);
    
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(GPUAssetInstance), (void*)0);
    glVertexAttribDivisor(1, 1);
    
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(GPUAssetInstance), (void*)(2 * sizeof(float)));
    glVertexAttribDivisor(2, 1);
    
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(GPUAssetInstance), (void*)(3 * sizeof(float)));
    glVertexAttribDivisor(3, 1);
    
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(GPUAssetInstance), (void*)(4 * sizeof(float)));
    glVertexAttribDivisor(4, 1);
    
    glEnableVertexAttribArray(5);
    glVertexAttribPointer(5, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(GPUAssetInstance), (void*)(5 * sizeof(float)));
    glVertexAttribDivisor(5, 1);
    
    glEnableVertexAttribArray(6);
    glVertexAttribPointer(6, 1, GL_FLOAT, GL_FALSE, sizeof(GPUAssetInstance), (void*)(6 * sizeof(float)));
    glVertexAttribDivisor(6, 1);
    
    glEnableVertexAttribArray(7);
    glVertexAttribPointer(7, 1, GL_FLOAT, GL_FALSE, sizeof(GPUAssetInstance), (void*)(7 * sizeof(float)));
    glVertexAttribDivisor(7, 1);
    
    glEnableVertexAttribArray(8);
    glVertexAttribPointer(8, 1, GL_UNSIGNED_INT, GL_FALSE, sizeof(GPUAssetInstance), (void*)(8 * sizeof(float)));
    glVertexAttribDivisor(8, 1);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, assetCount);
    
    glDisable(GL_BLEND);
    
    glBindVertexArray(0);
    glUseProgram(0);
}

void InstancedAssetRenderer::setPointSize(float size) {
    point_size_ = size;
}

float InstancedAssetRenderer::getPointSize() const {
    return point_size_;
}

void InstancedAssetRenderer::setGlowEnabled(bool enabled) {
    glow_enabled_ = enabled;
}

bool InstancedAssetRenderer::isGlowEnabled() const {
    return glow_enabled_;
}

void InstancedAssetRenderer::setProjection(float left, float right, float bottom, float top) {
    projection_[0] = 2.0f / (right - left);
    projection_[3] = -(right + left) / (right - left);
    projection_[5] = 2.0f / (top - bottom);
    projection_[7] = -(top + bottom) / (top - bottom);
}