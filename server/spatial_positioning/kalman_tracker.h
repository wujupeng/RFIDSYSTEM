#pragma once

#include "spatial_types.h"

class KalmanTracker {
public:
    KalmanTracker();
    
    void init(double x, double y, double vx = 0, double vy = 0);
    
    TagPosition update(double z_x, double z_y, double dt = 0.1);
    
    TagPosition getState() const;
    
    void reset();
    
private:
    double x_;
    double y_;
    double vx_;
    double vy_;
    
    double P[4][4];
    double Q[4][4];
    double R[2][2];
};