#include "decision_overlay_pass.h"
#include <cstring>

#ifdef _WIN32
#include <windows.h>
#include <GL/gl.h>
#else
#include <GL/gl.h>
#endif

const char* overlayVertexShader = R"(
#version 330 core

layout(location = 0) in vec2 quadPos;
layout(location = 1) in vec2 instancePos;
layout(location = 2) in float confidence;
layout(location = 3) in uint action;
layout(location = 4) in vec4 color;
layout(location = 5) in float pulse;

uniform mat4 projection;
uniform float currentTime;

out float vConfidence;
out uint vAction;
out vec4 vColor;

void main() {
    vec2 pos = quadPos * (10.0 + confidence * 5.0) + instancePos;
    gl_Position = projection * vec4(pos, 0.0, 1.0);
    
    vConfidence = confidence;
    vAction = action;
    vColor = color;
}
)";

const char* overlayFragmentShader = R"(
#version 330 core

in float vConfidence;
in uint vAction;
in vec4 vColor;

uniform float currentTime;

out vec4 fragColor;

void main() {
    vec2 center = gl_PointCoord - vec2(0.5);
    float dist = length(center);
    
    if (dist > 0.5) {
        discard;
    }
    
    float shapeAlpha = 1.0 - smoothstep(0.3, 0.5, dist);
    
    float pulse = 0.5 + 0.5 * sin(currentTime * 4.0);
    
    float alpha = shapeAlpha * vConfidence * pulse;
    
    vec3 color = vColor.rgb;
    
    if (vAction == 1) {
        color = mix(color, vec3(1.0, 0.2, 0.2), 0.5);
    } else if (vAction == 2) {
        color = mix(color, vec3(1.0, 0.8, 0.2), 0.5);
    } else if (vAction == 3) {
        color = mix(color, vec3(0.2, 0.5, 1.0), 0.5);
    }
    
    fragColor = vec4(color, alpha);
}
)";

DecisionOverlayPass::DecisionOverlayPass()
    : RenderPass("DecisionOverlayPass", PassType::CONGESTION),
      shader_program_(0), vao_(0), vbo_(0), buffer_(nullptr),
      current_time_(0.0f), initialized_(false) {
    
    memset(projection_, 0, sizeof(projection_));
    projection_[0] = 1.0f;
    projection_[5] = 1.0f;
    projection_[10] = 1.0f;
    projection_[15] = 1.0f;
}

DecisionOverlayPass::~DecisionOverlayPass() {
    shutdown();
}

void DecisionOverlayPass::initialize() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (initialized_) {
        return;
    }
    
    createShaders();
    
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
    
    initialized_ = true;
}

void DecisionOverlayPass::shutdown() {
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

void DecisionOverlayPass::createShaders() {
    uint32_t vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &overlayVertexShader, nullptr);
    glCompileShader(vertexShader);
    
    uint32_t fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &overlayFragmentShader, nullptr);
    glCompileShader(fragmentShader);
    
    shader_program_ = glCreateProgram();
    glAttachShader(shader_program_, vertexShader);
    glAttachShader(shader_program_, fragmentShader);
    glLinkProgram(shader_program_);
    
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

void DecisionOverlayPass::execute() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_ || !buffer_) {
        return;
    }
    
    uint32_t bufferId = buffer_->getBufferId();
    size_t overlayCount = buffer_->getOverlayCount();
    
    if (bufferId == 0 || overlayCount == 0) {
        return;
    }
    
    glUseProgram(shader_program_);
    
    int projLoc = glGetUniformLocation(shader_program_, "projection");
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, projection_);
    
    int timeLoc = glGetUniformLocation(shader_program_, "currentTime");
    glUniform1f(timeLoc, current_time_);
    
    glBindVertexArray(vao_);
    
    glBindBuffer(GL_ARRAY_BUFFER, bufferId);
    
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(GPUDecisionOverlay), (void*)0);
    glVertexAttribDivisor(1, 1);
    
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(GPUDecisionOverlay), (void*)(2 * sizeof(float)));
    glVertexAttribDivisor(2, 1);
    
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 1, GL_UNSIGNED_INT, GL_FALSE, sizeof(GPUDecisionOverlay), (void*)(3 * sizeof(float)));
    glVertexAttribDivisor(3, 1);
    
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(GPUDecisionOverlay), (void*)(4 * sizeof(float)));
    glVertexAttribDivisor(4, 1);
    
    glEnableVertexAttribArray(5);
    glVertexAttribPointer(5, 1, GL_FLOAT, GL_FALSE, sizeof(GPUDecisionOverlay), (void*)(5 * sizeof(float)));
    glVertexAttribDivisor(5, 1);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, static_cast<int>(overlayCount));
    
    glDisable(GL_BLEND);
    
    glBindVertexArray(0);
    glUseProgram(0);
}

void DecisionOverlayPass::setInput(uint32_t textureId) {
}

uint32_t DecisionOverlayPass::getOutput() const {
    return 0;
}

void DecisionOverlayPass::setOverlayBuffer(const DecisionOverlayBuffer* buffer) {
    buffer_ = buffer;
}

void DecisionOverlayPass::setCurrentTime(float time) {
    current_time_ = time;
}