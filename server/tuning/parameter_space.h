#pragma once

#include <string>
#include <vector>
#include <unordered_map>

struct TunableParameter {
    std::string name;
    double min_value;
    double max_value;
    double step;
    double current_value;
    double default_value;
};

class ParameterSpace {
public:
    static ParameterSpace& instance();

    std::vector<TunableParameter> getAllParameters() const;

    void initializeDefaults();

    void updateParameter(const std::string& name, double value);

    std::unordered_map<std::string, double> snapshot();

    void restore(const std::unordered_map<std::string, double>& params);

    double getParameter(const std::string& name) const;

    void resetToDefaults();

private:
    ParameterSpace();

    std::vector<TunableParameter> parameters_;
};
