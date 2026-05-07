#pragma once

#include <QObject>
#include <QOpenGLFunctions>
#include <QMatrix4x4>
#include <QVector>
#include <QMutex>

struct GpuAssetInstance {
    float x;
    float y;
    float vx;
    float vy;
    float risk;
    float state;
    float size;
    float heat;
};

struct TrailPoint {
    QVector2D pos;
    float timestamp;
};

struct HeatCell {
    int x;
    int y;
    float value;
};

struct CongestionZone {
    int id;
    float x;
    float y;
    float radius;
    float score;
    QString label;
};

class SpatialRenderer : public QObject, protected QOpenGLFunctions {
    Q_OBJECT
    
public:
    explicit SpatialRenderer(QObject* parent = nullptr);
    ~SpatialRenderer();
    
    void initialize();
    void resize(int width, int height);
    void render();
    void cleanup();
    
    void updateAssets(const QVector<GpuAssetInstance>& assets);
    void updateHeatmap(const QVector<HeatCell>& heatmap);
    void updateCongestion(const QVector<CongestionZone>& zones);
    void updateTrails(int assetId, const QVector<TrailPoint>& trails);
    
    void setViewMatrix(const QMatrix4x4& matrix);
    void setProjectionMatrix(const QMatrix4x4& matrix);
    
signals:
    void initialized();
    void rendered();
    
private:
    void initParticleShader();
    void initHeatmapShader();
    void initTrajectoryShader();
    
    void renderParticles();
    void renderHeatmap();
    void renderTrajectories();
    void renderCongestion();
    
    QMutex mutex;
    
    // Particle rendering
    GLuint particleProgram;
    GLuint particleVAO;
    GLuint particleVBO;
    GLuint particleInstanceVBO;
    QVector<GpuAssetInstance> assetInstances;
    
    // Heatmap rendering
    GLuint heatmapProgram;
    GLuint heatmapTexture;
    GLuint heatmapVAO;
    int heatmapSize;
    
    // Trajectory rendering
    GLuint trajectoryProgram;
    GLuint trailVAO;
    QMap<int, QVector<TrailPoint>> trails;
    
    // Congestion rendering
    GLuint congestionProgram;
    QVector<CongestionZone> congestionZones;
    
    // Matrices
    QMatrix4x4 viewMatrix;
    QMatrix4x4 projectionMatrix;
    QMatrix4x4 modelMatrix;
    
    // Screen dimensions
    int screenWidth;
    int screenHeight;
    
    // Shader locations
    int particleModelMatrixLoc;
    int particleViewMatrixLoc;
    int particleProjMatrixLoc;
    int particleTimeLoc;
    
    int heatmapModelMatrixLoc;
    int heatmapProjMatrixLoc;
    int heatmapTextureLoc;
    
    int trailModelMatrixLoc;
    int trailViewMatrixLoc;
    int trailProjMatrixLoc;
    int trailTimeLoc;
};