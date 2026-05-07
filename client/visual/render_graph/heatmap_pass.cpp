#include "heatmap_pass.h"
#include "../gpu/gpu_frame_uploader.h"
#include <cstring>

#ifdef _WIN32
#include <windows.h>
#include <GL/gl.h>
#else
#include <GL/gl.h>
#endif

const char* splatVertexShader = R"(
#version 330 core
layout(location = 0) in vec2 pos;
void main() { gl_Position = vec4(pos, 0.0, 1.0); }
)";

const char* splatFragmentShader = R"(
#version 330 core
out vec4 fragColor;
void main() { fragColor = vec4(1.0); }
)";

const char* blurVertexShader = R"(
#version 330 core
layout(location = 0) in vec2 pos;
out vec2 texCoord;
void main() {
    texCoord = (pos + 1.0) / 2.0;
    gl_Position = vec4(pos, 0.0, 1.0);
}
)";

const char* blurFragmentShader = R"(
#version 330 core
in vec2 texCoord;
uniform sampler2D inputTexture;
uniform bool horizontal;
out vec4 fragColor;
void main() {
    vec2 offset = horizontal ? vec2(1.0 / textureSize(inputTexture, 0).x, 0.0) : vec2(0.0, 1.0 / textureSize(inputTexture, 0).y);
    vec3 color = vec3(0.0);
    float weight[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);
    for(int i = -2; i <= 2; i++) {
        color += texture(inputTexture, texCoord + offset * float(i)).rgb * weight[i + 2];
    }
    fragColor = vec4(color, 1.0);
}
)";

const char* colorRampFragmentShader = R"(
#version 330 core
in vec2 texCoord;
uniform sampler2D inputTexture;
out vec4 fragColor;
void main() {
    float value = texture(inputTexture, texCoord).r;
    vec3 color = vec3(0.0, 0.2, 1.0);
    if(value > 0.3) color = mix(vec3(0.0, 0.5, 1.0), vec3(0.0, 1.0, 0.5), (value - 0.3) / 0.2);
    if(value > 0.5) color = mix(vec3(0.0, 1.0, 0.5), vec3(1.0, 1.0, 0.0), (value - 0.5) / 0.3);
    if(value > 0.8) color = mix(vec3(1.0, 1.0, 0.0), vec3(1.0, 0.0, 0.0), (value - 0.8) / 0.2);
    fragColor = vec4(color, value * 0.6);
}
)";

HeatmapPass::HeatmapPass()
    : RenderPass("HeatmapPass", PassType::HEATMAP),
      splat_shader_(0), blur_shader_(0), color_ramp_shader_(0),
      splat_fbo_(0), blur_fbo_(0), output_fbo_(0),
      splat_texture_(0), blur_texture_(0), output_texture_(0),
      uploader_(nullptr), texture_size_(512), initialized_(false) {
}

HeatmapPass::~HeatmapPass() {
    shutdown();
}

void HeatmapPass::initialize() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (initialized_) {
        return;
    }
    
    createShaders();
    
    glGenFramebuffers(1, &splat_fbo_);
    glGenFramebuffers(1, &blur_fbo_);
    glGenFramebuffers(1, &output_fbo_);
    
    glGenTextures(1, &splat_texture_);
    glBindTexture(GL_TEXTURE_2D, splat_texture_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, texture_size_, texture_size_, 0, GL_RED, GL_FLOAT, nullptr);
    
    glGenTextures(1, &blur_texture_);
    glBindTexture(GL_TEXTURE_2D, blur_texture_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, texture_size_, texture_size_, 0, GL_RED, GL_FLOAT, nullptr);
    
    glGenTextures(1, &output_texture_);
    glBindTexture(GL_TEXTURE_2D, output_texture_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, texture_size_, texture_size_, 0, GL_RGBA, GL_FLOAT, nullptr);
    
    initialized_ = true;
}

void HeatmapPass::shutdown() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        return;
    }
    
    glDeleteFramebuffers(1, &splat_fbo_);
    glDeleteFramebuffers(1, &blur_fbo_);
    glDeleteFramebuffers(1, &output_fbo_);
    
    glDeleteTextures(1, &splat_texture_);
    glDeleteTextures(1, &blur_texture_);
    glDeleteTextures(1, &output_texture_);
    
    glDeleteProgram(splat_shader_);
    glDeleteProgram(blur_shader_);
    glDeleteProgram(color_ramp_shader_);
    
    initialized_ = false;
}

void HeatmapPass::createShaders() {
    uint32_t vs = glCreateShader(GL_VERTEX_SHADER);
    uint32_t fs = glCreateShader(GL_FRAGMENT_SHADER);
    
    glShaderSource(vs, 1, &splatVertexShader, nullptr);
    glCompileShader(vs);
    glShaderSource(fs, 1, &splatFragmentShader, nullptr);
    glCompileShader(fs);
    splat_shader_ = glCreateProgram();
    glAttachShader(splat_shader_, vs);
    glAttachShader(splat_shader_, fs);
    glLinkProgram(splat_shader_);
    
    glShaderSource(vs, 1, &blurVertexShader, nullptr);
    glCompileShader(vs);
    glShaderSource(fs, 1, &blurFragmentShader, nullptr);
    glCompileShader(fs);
    blur_shader_ = glCreateProgram();
    glAttachShader(blur_shader_, vs);
    glAttachShader(blur_shader_, fs);
    glLinkProgram(blur_shader_);
    
    glShaderSource(vs, 1, &blurVertexShader, nullptr);
    glCompileShader(vs);
    glShaderSource(fs, 1, &colorRampFragmentShader, nullptr);
    glCompileShader(fs);
    color_ramp_shader_ = glCreateProgram();
    glAttachShader(color_ramp_shader_, vs);
    glAttachShader(color_ramp_shader_, fs);
    glLinkProgram(color_ramp_shader_);
    
    glDeleteShader(vs);
    glDeleteShader(fs);
}

void HeatmapPass::execute() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_ || !uploader_) {
        return;
    }
    
    glBindFramebuffer(GL_FRAMEBUFFER, splat_fbo_);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, splat_texture_, 0);
    
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    
    glUseProgram(splat_shader_);
    
    uint32_t heatmapBuffer = uploader_->getHeatmapBufferId();
    uint32_t heatmapCount = uploader_->getHeatmapCount();
    
    if (heatmapBuffer && heatmapCount > 0) {
        glBindBuffer(GL_ARRAY_BUFFER, heatmapBuffer);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(GPUHeatCell), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(GPUHeatCell), (void*)(2 * sizeof(float)));
        
        glPointSize(8.0f);
        glDrawArrays(GL_POINTS, 0, heatmapCount);
    }
    
    glDisable(GL_BLEND);
    
    blurTexture(splat_texture_, blur_texture_);
    
    glBindFramebuffer(GL_FRAMEBUFFER, output_fbo_);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, output_texture_, 0);
    
    glUseProgram(color_ramp_shader_);
    glUniform1i(glGetUniformLocation(color_ramp_shader_, "inputTexture"), 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, blur_texture_);
    
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void HeatmapPass::blurTexture(uint32_t input, uint32_t output) {
    glBindFramebuffer(GL_FRAMEBUFFER, blur_fbo_);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, output, 0);
    
    glUseProgram(blur_shader_);
    glUniform1i(glGetUniformLocation(blur_shader_, "inputTexture"), 0);
    glUniform1i(glGetUniformLocation(blur_shader_, "horizontal"), true);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, input);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, input, 0);
    glUniform1i(glGetUniformLocation(blur_shader_, "horizontal"), false);
    glBindTexture(GL_TEXTURE_2D, output);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void HeatmapPass::setInput(uint32_t textureId) {
}

uint32_t HeatmapPass::getOutput() const {
    return output_texture_;
}

void HeatmapPass::setUploader(const GPUFrameUploader* uploader) {
    uploader_ = uploader;
}