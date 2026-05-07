#include "trail_renderer.h"
#include "trail_ring_buffer.h"
#include <cstring>

#ifdef _WIN32
#include <windows.h>
#include <GL/gl.h>
#else
#include <GL/gl.h>
#endif

const char* trailVertexShader = R"(
#version 330 core

layout(location = 0) in vec2 pos;
layout(location = 1) in float timestamp;
layout(location = 2) in float intensity;
layout(location = 3) in uint color;

uniform mat4 projection;
uniform float currentTime;
uniform float decayFactor;
uniform float maxAge;

out float vAlpha;
out vec4 vColor;

void main() {
    gl_Position = projection * vec4(pos, 0.0, 1.0);
    
    float age = currentTime - timestamp;
    float alpha = exp(-age * decayFactor);
    
    alpha = clamp(alpha, 0.0, 1.0);
    
    vAlpha = alpha * intensity;
    
    float r = ((color >> 24) & 0xFF) / 255.0;
    float g = ((color >> 16) & 0xFF) / 255.0;
    float b = ((color >> 8) & 0xFF) / 255.0;
    float a = (color & 0xFF) / 255.0;
    
    vColor = vec4(r, g, b, a);
}
)";

const char* trailFragmentShader = R"(
#version 330 core

in float vAlpha;
in vec4 vColor;

out vec4 fragColor;

void main() {
    vec2 center = gl_PointCoord - vec2(0.5);
    float dist = length(center);
    
    if (dist > 0.5) {
        discard;
    }
    
    float shapeAlpha = 1.0 - smoothstep(0.2, 0.5, dist);
    float finalAlpha = vAlpha * shapeAlpha;
    
    fragColor = vec4(vColor.rgb, finalAlpha);
}
)";

TrailRenderer::TrailRenderer()
    : shader_program_(0), vao_(0), vbo_(0),
      decay_factor_(0.1f), max_age_(10.0f),
      initialized_(false) {
    
    memset(projection_, 0, sizeof(projection_));
    projection_[0] = 1.0f;
    projection_[5] = 1.0f;
    projection_[10] = 1.0f;
    projection_[15] = 1.0f;
}

TrailRenderer::~TrailRenderer() {
    shutdown();
}

void TrailRenderer::initialize() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (initialized_) {
        return;
    }
    
    createShaders();
    createGeometry();
    
    initialized_ = true;
}

void TrailRenderer::shutdown() {
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

void TrailRenderer::createShaders() {
    uint32_t vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &trailVertexShader, nullptr);
    glCompileShader(vertexShader);
    
    uint32_t fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &trailFragmentShader, nullptr);
    glCompileShader(fragmentShader);
    
    shader_program_ = glCreateProgram();
    glAttachShader(shader_program_, vertexShader);
    glAttachShader(shader_program_, fragmentShader);
    glLinkProgram(shader_program_);
    
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

void TrailRenderer::createGeometry() {
    glGenVertexArrays(1, &vao_);
    glBindVertexArray(vao_);
    
    glGenBuffers(1, &vbo_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    
    glBindVertexArray(0);
}

void TrailRenderer::render(const TrailRingBuffer& buffer, float currentTime,
                          float viewX, float viewY, float viewWidth, float viewHeight) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        return;
    }
    
    uint32_t bufferId = buffer.getBufferId();
    size_t pointCount = buffer.getPointCount();
    
    if (bufferId == 0 || pointCount == 0) {
        return;
    }
    
    glUseProgram(shader_program_);
    
    int projLoc = glGetUniformLocation(shader_program_, "projection");
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, projection_);
    
    int timeLoc = glGetUniformLocation(shader_program_, "currentTime");
    glUniform1f(timeLoc, currentTime);
    
    int decayLoc = glGetUniformLocation(shader_program_, "decayFactor");
    glUniform1f(decayLoc, decay_factor_);
    
    int ageLoc = glGetUniformLocation(shader_program_, "maxAge");
    glUniform1f(ageLoc, max_age_);
    
    glBindVertexArray(vao_);
    
    glBindBuffer(GL_ARRAY_BUFFER, bufferId);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(GPUTrailPoint), (void*)0);
    
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(GPUTrailPoint), (void*)(2 * sizeof(float)));
    
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(GPUTrailPoint), (void*)(3 * sizeof(float)));
    
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 1, GL_UNSIGNED_INT, GL_FALSE, sizeof(GPUTrailPoint), (void*)(4 * sizeof(float)));
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glPointSize(3.0f);
    
    glDrawArrays(GL_POINTS, 0, static_cast<int>(pointCount));
    
    glDisable(GL_BLEND);
    
    glBindVertexArray(0);
    glUseProgram(0);
}

void TrailRenderer::setProjection(float left, float right, float bottom, float top) {
    projection_[0] = 2.0f / (right - left);
    projection_[3] = -(right + left) / (right - left);
    projection_[5] = 2.0f / (top - bottom);
    projection_[7] = -(top + bottom) / (top - bottom);
}

void TrailRenderer::setDecayFactor(float factor) {
    decay_factor_ = factor;
}

void TrailRenderer::setMaxAge(float age) {
    max_age_ = age;
}