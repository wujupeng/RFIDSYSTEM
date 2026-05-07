#include "spatial_renderer.h"
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QDebug>

const char* particleVertexShader = R"(
    #version 330 core
    layout (location = 0) in vec2 aPos;
    
    uniform mat4 model;
    uniform mat4 view;
    uniform mat4 projection;
    uniform float time;
    
    in float x;
    in float y;
    in float vx;
    in float vy;
    in float risk;
    in float state;
    in float size;
    in float heat;
    
    out float v_risk;
    out float v_state;
    out float v_heat;
    out float v_pulse;
    
    void main() {
        vec4 worldPos = model * vec4(x + aPos.x * size, y + aPos.y * size, 0.0, 1.0);
        gl_Position = projection * view * worldPos;
        
        v_risk = risk;
        v_state = state;
        v_heat = heat;
        v_pulse = 0.5 + 0.5 * sin(time * 3.0 + x + y);
    }
)";

const char* particleFragmentShader = R"(
    #version 330 core
    in float v_risk;
    in float v_state;
    in float v_heat;
    in float v_pulse;
    
    out vec4 FragColor;
    
    void main() {
        vec2 coord = gl_PointCoord - vec2(0.5);
        float dist = length(coord);
        
        if (dist > 0.5) discard;
        
        float alpha = 1.0 - smoothstep(0.3, 0.5, dist);
        
        vec3 color;
        if (v_state == 3.0) {
            color = vec3(1.0, 0.23, 0.19);
        } else if (v_state == 2.0) {
            color = vec3(1.0, 0.69, 0.13);
        } else if (v_state == 1.0) {
            color = vec3(0.75, 0.0, 1.0);
        } else {
            color = mix(vec3(0.0, 0.83, 1.0), vec3(1.0, 0.69, 0.13), v_risk);
        }
        
        color = mix(color, vec3(1.0), v_pulse * 0.2);
        
        FragColor = vec4(color, alpha * v_pulse);
    }
)";

const char* heatmapVertexShader = R"(
    #version 330 core
    layout (location = 0) in vec2 aPos;
    
    uniform mat4 model;
    uniform mat4 projection;
    
    out vec2 TexCoords;
    
    void main() {
        gl_Position = projection * model * vec4(aPos, 0.0, 1.0);
        TexCoords = (aPos + vec2(1.0)) / 2.0;
    }
)";

const char* heatmapFragmentShader = R"(
    #version 330 core
    in vec2 TexCoords;
    
    uniform sampler2D heatmap;
    
    out vec4 FragColor;
    
    void main() {
        float heat = texture(heatmap, TexCoords).r;
        
        vec3 color;
        if (heat < 0.2) {
            color = mix(vec3(0.0, 0.13, 0.38), vec3(0.0, 0.83, 1.0), heat * 5.0);
        } else if (heat < 0.5) {
            color = mix(vec3(0.0, 0.83, 1.0), vec3(1.0, 1.0, 0.0), (heat - 0.2) * 3.33);
        } else if (heat < 0.8) {
            color = mix(vec3(1.0, 1.0, 0.0), vec3(1.0, 0.69, 0.13), (heat - 0.5) * 3.33);
        } else {
            color = mix(vec3(1.0, 0.69, 0.13), vec3(1.0, 0.23, 0.19), (heat - 0.8) * 5.0);
        }
        
        FragColor = vec4(color, heat * 0.6);
    }
)";

const char* trajectoryVertexShader = R"(
    #version 330 core
    layout (location = 0) in vec3 aPos;
    layout (location = 1) in float aTime;
    
    uniform mat4 model;
    uniform mat4 view;
    uniform mat4 projection;
    uniform float currentTime;
    
    out float v_alpha;
    
    void main() {
        vec4 worldPos = model * vec4(aPos.x, aPos.y, 0.0, 1.0);
        gl_Position = projection * view * worldPos;
        
        float delta = currentTime - aTime;
        v_alpha = exp(-delta * 0.5);
    }
)";

const char* trajectoryFragmentShader = R"(
    #version 330 core
    in float v_alpha;
    
    out vec4 FragColor;
    
    void main() {
        FragColor = vec4(0.4, 0.8, 1.0, v_alpha * 0.8);
    }
)";

SpatialRenderer::SpatialRenderer(QObject* parent) 
    : QObject(parent), screenWidth(1920), screenHeight(1080), heatmapSize(1024) {
}

SpatialRenderer::~SpatialRenderer() {
    cleanup();
}

void SpatialRenderer::initialize() {
    initializeOpenGLFunctions();
    
    initParticleShader();
    initHeatmapShader();
    initTrajectoryShader();
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);
    
    emit initialized();
}

void SpatialRenderer::resize(int width, int height) {
    screenWidth = width;
    screenHeight = height;
    
    projectionMatrix.setToIdentity();
    projectionMatrix.ortho(0, width, height, 0, -1, 1);
}

void SpatialRenderer::render() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.05, 0.06, 0.13, 1.0);
    
    renderHeatmap();
    renderTrajectories();
    renderParticles();
    renderCongestion();
    
    emit rendered();
}

void SpatialRenderer::cleanup() {
    if (particleVAO) glDeleteVertexArrays(1, &particleVAO);
    if (particleVBO) glDeleteBuffers(1, &particleVBO);
    if (particleInstanceVBO) glDeleteBuffers(1, &particleInstanceVBO);
    if (heatmapTexture) glDeleteTextures(1, &heatmapTexture);
    if (heatmapVAO) glDeleteVertexArrays(1, &heatmapVAO);
    if (trailVAO) glDeleteVertexArrays(1, &trailVAO);
}

void SpatialRenderer::initParticleShader() {
    particleProgram = glCreateProgram();
    
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &particleVertexShader, nullptr);
    glCompileShader(vertexShader);
    
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &particleFragmentShader, nullptr);
    glCompileShader(fragmentShader);
    
    glAttachShader(particleProgram, vertexShader);
    glAttachShader(particleProgram, fragmentShader);
    glLinkProgram(particleProgram);
    
    particleModelMatrixLoc = glGetUniformLocation(particleProgram, "model");
    particleViewMatrixLoc = glGetUniformLocation(particleProgram, "view");
    particleProjMatrixLoc = glGetUniformLocation(particleProgram, "projection");
    particleTimeLoc = glGetUniformLocation(particleProgram, "time");
    
    float vertices[] = {
        -1.0f, -1.0f,
         1.0f, -1.0f,
         1.0f,  1.0f,
        -1.0f,  1.0f
    };
    
    glGenVertexArrays(1, &particleVAO);
    glBindVertexArray(particleVAO);
    
    glGenBuffers(1, &particleVBO);
    glBindBuffer(GL_ARRAY_BUFFER, particleVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    glGenBuffers(1, &particleInstanceVBO);
}

void SpatialRenderer::initHeatmapShader() {
    heatmapProgram = glCreateProgram();
    
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &heatmapVertexShader, nullptr);
    glCompileShader(vertexShader);
    
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &heatmapFragmentShader, nullptr);
    glCompileShader(fragmentShader);
    
    glAttachShader(heatmapProgram, vertexShader);
    glAttachShader(heatmapProgram, fragmentShader);
    glLinkProgram(heatmapProgram);
    
    heatmapModelMatrixLoc = glGetUniformLocation(heatmapProgram, "model");
    heatmapProjMatrixLoc = glGetUniformLocation(heatmapProgram, "projection");
    heatmapTextureLoc = glGetUniformLocation(heatmapProgram, "heatmap");
    
    glGenTextures(1, &heatmapTexture);
    glBindTexture(GL_TEXTURE_2D, heatmapTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    float* emptyData = new float[heatmapSize * heatmapSize]();
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, heatmapSize, heatmapSize, 0, GL_RED, GL_FLOAT, emptyData);
    delete[] emptyData;
    
    float quadVertices[] = {
        -1.0f, -1.0f,
         1.0f, -1.0f,
         1.0f,  1.0f,
        -1.0f,  1.0f
    };
    
    glGenVertexArrays(1, &heatmapVAO);
    glBindVertexArray(heatmapVAO);
    
    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
}

void SpatialRenderer::initTrajectoryShader() {
    trajectoryProgram = glCreateProgram();
    
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &trajectoryVertexShader, nullptr);
    glCompileShader(vertexShader);
    
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &trajectoryFragmentShader, nullptr);
    glCompileShader(fragmentShader);
    
    glAttachShader(trajectoryProgram, vertexShader);
    glAttachShader(trajectoryProgram, fragmentShader);
    glLinkProgram(trajectoryProgram);
    
    trailModelMatrixLoc = glGetUniformLocation(trajectoryProgram, "model");
    trailViewMatrixLoc = glGetUniformLocation(trajectoryProgram, "view");
    trailProjMatrixLoc = glGetUniformLocation(trajectoryProgram, "projection");
    trailTimeLoc = glGetUniformLocation(trajectoryProgram, "currentTime");
    
    glGenVertexArrays(1, &trailVAO);
}

void SpatialRenderer::updateAssets(const QVector<GpuAssetInstance>& assets) {
    QMutexLocker locker(&mutex);
    assetInstances = assets;
}

void SpatialRenderer::updateHeatmap(const QVector<HeatCell>& heatmap) {
    QMutexLocker locker(&mutex);
    
    float* heatData = new float[heatmapSize * heatmapSize]();
    
    for (const HeatCell& cell : heatmap) {
        if (cell.x >= 0 && cell.x < heatmapSize && cell.y >= 0 && cell.y < heatmapSize) {
            heatData[cell.y * heatmapSize + cell.x] = cell.value;
        }
    }
    
    glBindTexture(GL_TEXTURE_2D, heatmapTexture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, heatmapSize, heatmapSize, GL_RED, GL_FLOAT, heatData);
    
    delete[] heatData;
}

void SpatialRenderer::updateCongestion(const QVector<CongestionZone>& zones) {
    QMutexLocker locker(&mutex);
    congestionZones = zones;
}

void SpatialRenderer::updateTrails(int assetId, const QVector<TrailPoint>& trail) {
    QMutexLocker locker(&mutex);
    trails[assetId] = trail;
}

void SpatialRenderer::setViewMatrix(const QMatrix4x4& matrix) {
    viewMatrix = matrix;
}

void SpatialRenderer::setProjectionMatrix(const QMatrix4x4& matrix) {
    projectionMatrix = matrix;
}

void SpatialRenderer::renderParticles() {
    QMutexLocker locker(&mutex);
    
    if (assetInstances.isEmpty()) return;
    
    glUseProgram(particleProgram);
    
    glUniformMatrix4fv(particleModelMatrixLoc, 1, GL_FALSE, modelMatrix.data());
    glUniformMatrix4fv(particleViewMatrixLoc, 1, GL_FALSE, viewMatrix.data());
    glUniformMatrix4fv(particleProjMatrixLoc, 1, GL_FALSE, projectionMatrix.data());
    glUniform1f(particleTimeLoc, static_cast<float>(QDateTime::currentMSecsSinceEpoch()) / 1000.0f);
    
    glBindVertexArray(particleVAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, particleInstanceVBO);
    glBufferData(GL_ARRAY_BUFFER, assetInstances.size() * sizeof(GpuAssetInstance), 
                 assetInstances.constData(), GL_DYNAMIC_DRAW);
    
    int offset = 0;
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(GpuAssetInstance), (void*)offset);
    glEnableVertexAttribArray(1);
    glVertexAttribDivisor(1, 1);
    offset += sizeof(float);
    
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(GpuAssetInstance), (void*)offset);
    glEnableVertexAttribArray(2);
    glVertexAttribDivisor(2, 1);
    offset += sizeof(float);
    
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(GpuAssetInstance), (void*)offset);
    glEnableVertexAttribArray(3);
    glVertexAttribDivisor(3, 1);
    offset += sizeof(float);
    
    glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(GpuAssetInstance), (void*)offset);
    glEnableVertexAttribArray(4);
    glVertexAttribDivisor(4, 1);
    offset += sizeof(float);
    
    glVertexAttribPointer(5, 1, GL_FLOAT, GL_FALSE, sizeof(GpuAssetInstance), (void*)offset);
    glEnableVertexAttribArray(5);
    glVertexAttribDivisor(5, 1);
    offset += sizeof(float);
    
    glVertexAttribPointer(6, 1, GL_FLOAT, GL_FALSE, sizeof(GpuAssetInstance), (void*)offset);
    glEnableVertexAttribArray(6);
    glVertexAttribDivisor(6, 1);
    offset += sizeof(float);
    
    glVertexAttribPointer(7, 1, GL_FLOAT, GL_FALSE, sizeof(GpuAssetInstance), (void*)offset);
    glEnableVertexAttribArray(7);
    glVertexAttribDivisor(7, 1);
    offset += sizeof(float);
    
    glVertexAttribPointer(8, 1, GL_FLOAT, GL_FALSE, sizeof(GpuAssetInstance), (void*)offset);
    glEnableVertexAttribArray(8);
    glVertexAttribDivisor(8, 1);
    
    glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 4, assetInstances.size());
}

void SpatialRenderer::renderHeatmap() {
    glUseProgram(heatmapProgram);
    
    QMatrix4x4 model;
    model.scale(screenWidth / 2.0f, screenHeight / 2.0f);
    model.translate(1.0f, 1.0f);
    
    glUniformMatrix4fv(heatmapModelMatrixLoc, 1, GL_FALSE, model.data());
    glUniformMatrix4fv(heatmapProjMatrixLoc, 1, GL_FALSE, projectionMatrix.data());
    glUniform1i(heatmapTextureLoc, 0);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, heatmapTexture);
    
    glBindVertexArray(heatmapVAO);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

void SpatialRenderer::renderTrajectories() {
    QMutexLocker locker(&mutex);
    
    if (trails.isEmpty()) return;
    
    glUseProgram(trajectoryProgram);
    
    glUniformMatrix4fv(trailModelMatrixLoc, 1, GL_FALSE, modelMatrix.data());
    glUniformMatrix4fv(trailViewMatrixLoc, 1, GL_FALSE, viewMatrix.data());
    glUniformMatrix4fv(trailProjMatrixLoc, 1, GL_FALSE, projectionMatrix.data());
    glUniform1f(trailTimeLoc, static_cast<float>(QDateTime::currentMSecsSinceEpoch()) / 1000.0f);
    
    glBindVertexArray(trailVAO);
    
    for (const auto& entry : trails) {
        const QVector<TrailPoint>& trail = entry.second;
        if (trail.size() < 2) continue;
        
        QVector<float> vertexData;
        vertexData.reserve(trail.size() * 3);
        
        for (const TrailPoint& point : trail) {
            vertexData.append(point.pos.x());
            vertexData.append(point.pos.y());
            vertexData.append(point.timestamp);
        }
        
        GLuint vbo;
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(float), vertexData.constData(), GL_STREAM_DRAW);
        
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        
        glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)(2 * sizeof(float)));
        glEnableVertexAttribArray(1);
        
        glDrawArrays(GL_LINE_STRIP, 0, trail.size());
        
        glDeleteBuffers(1, &vbo);
    }
}

void SpatialRenderer::renderCongestion() {
    QMutexLocker locker(&mutex);
    
    if (congestionZones.isEmpty()) return;
    
    for (const CongestionZone& zone : congestionZones) {
        QColor color;
        if (zone.score < 0.3) {
            color = QColor(0, 255, 0, 100);
        } else if (zone.score < 0.6) {
            color = QColor(255, 255, 0, 120);
        } else if (zone.score < 0.9) {
            color = QColor(255, 165, 0, 150);
        } else {
            color = QColor(255, 0, 0, 180);
        }
        
        glColor4f(color.redF(), color.greenF(), color.blueF(), color.alphaF());
        glBegin(GL_TRIANGLE_FAN);
        
        for (int i = 0; i <= 32; ++i) {
            float angle = static_cast<float>(i) / 32.0f * 2.0f * static_cast<float>(M_PI);
            float x = zone.x + cos(angle) * zone.radius;
            float y = zone.y + sin(angle) * zone.radius;
            glVertex2f(x, y);
        }
        
        glEnd();
    }
}