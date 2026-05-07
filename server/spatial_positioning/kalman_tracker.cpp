#include "kalman_tracker.h"
#include <cmath>

KalmanTracker::KalmanTracker() {
    reset();
}

void KalmanTracker::reset() {
    x_ = 0;
    y_ = 0;
    vx_ = 0;
    vy_ = 0;
    
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            P[i][j] = (i == j) ? 100.0 : 0.0;
            Q[i][j] = (i == j) ? 0.1 : 0.0;
        }
    }
    
    R[0][0] = 1.0;
    R[0][1] = 0.0;
    R[1][0] = 0.0;
    R[1][1] = 1.0;
}

void KalmanTracker::init(double x, double y, double vx, double vy) {
    x_ = x;
    y_ = y;
    vx_ = vx;
    vy_ = vy;
}

TagPosition KalmanTracker::update(double z_x, double z_y, double dt) {
    double F[4][4] = {
        {1, 0, dt, 0},
        {0, 1, 0, dt},
        {0, 0, 1, 0},
        {0, 0, 0, 1}
    };
    
    double H[2][4] = {
        {1, 0, 0, 0},
        {0, 1, 0, 0}
    };
    
    double x_pred[4] = {x_, y_, vx_, vy_};
    double P_pred[4][4] = {0};
    
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            for (int k = 0; k < 4; ++k) {
                P_pred[i][j] += F[i][k] * P[k][j] * F[i][j];
            }
            P_pred[i][j] += Q[i][j];
        }
    }
    
    double S[2][2] = {0};
    double H_P[2][4] = {0};
    
    for (int i = 0; i < 2; ++i) {
        for (int j = 0; j < 4; ++j) {
            for (int k = 0; k < 4; ++k) {
                H_P[i][j] += H[i][k] * P_pred[k][j];
            }
        }
    }
    
    for (int i = 0; i < 2; ++i) {
        for (int j = 0; j < 2; ++j) {
            for (int k = 0; k < 4; ++k) {
                S[i][j] += H_P[i][k] * H[j][k];
            }
            S[i][j] += R[i][j];
        }
    }
    
    double S_inv[2][2];
    double det = S[0][0] * S[1][1] - S[0][1] * S[1][0];
    if (det != 0) {
        S_inv[0][0] = S[1][1] / det;
        S_inv[0][1] = -S[0][1] / det;
        S_inv[1][0] = -S[1][0] / det;
        S_inv[1][1] = S[0][0] / det;
    }
    
    double K[4][2] = {0};
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 2; ++j) {
            for (int k = 0; k < 2; ++k) {
                K[i][j] += P_pred[i][k] * H[j][k] * S_inv[k][j];
            }
        }
    }
    
    double z[2] = {z_x, z_y};
    double y[2] = {z[0] - x_pred[0], z[1] - x_pred[1]};
    
    x_ = x_pred[0] + K[0][0] * y[0] + K[0][1] * y[1];
    y_ = x_pred[1] + K[1][0] * y[0] + K[1][1] * y[1];
    vx_ = x_pred[2] + K[2][0] * y[0] + K[2][1] * y[1];
    vy_ = x_pred[3] + K[3][0] * y[0] + K[3][1] * y[1];
    
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            double sum = 0;
            for (int k = 0; k < 2; ++k) {
                sum += K[i][k] * H[k][j];
            }
            P[i][j] = P_pred[i][j] - sum * P_pred[i][j];
        }
    }
    
    TagPosition pos;
    pos.x = x_;
    pos.y = y_;
    pos.velocity_x = vx_;
    pos.velocity_y = vy_;
    pos.confidence = 0.9;
    
    return pos;
}

TagPosition KalmanTracker::getState() const {
    TagPosition pos;
    pos.x = x_;
    pos.y = y_;
    pos.velocity_x = vx_;
    pos.velocity_y = vy_;
    pos.confidence = 0.9;
    return pos;
}