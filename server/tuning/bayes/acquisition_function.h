#pragma once

enum class AcquisitionType {
    UCB,
    EI,
    PI,
    Thompson
};

class AcquisitionFunction {
public:
    AcquisitionFunction();

    double compute(double mean, double uncertainty, double beta = 2.0);

    double upperConfidenceBound(double mean, double uncertainty, double beta);

    double expectedImprovement(double mean, double uncertainty,
                               double best_reward, double epsilon = 0.01);

    double probabilityOfImprovement(double mean, double uncertainty,
                                   double best_reward, double epsilon = 0.01);

    void setType(AcquisitionType type) { type_ = type; }

    AcquisitionType getType() const { return type_; }

    void setBeta(double beta) { beta_ = beta; }

private:
    AcquisitionType type_;
    double beta_;
    double epsilon_;
};
