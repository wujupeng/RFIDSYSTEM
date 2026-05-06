#include "bandit_engine.h"
#include "../reward_evaluator.h"
#include "../decision_repository.h"
#include <fstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace bandit {

BanditEngine& BanditEngine::instance() {
    static BanditEngine instance;
    return instance;
}

BanditEngine::BanditEngine()
    : algorithm_(std::make_unique<LinUCBAlgorithm>(getFeatureDimension(), 1.0)),
      mode_(BanditMode::SHADOW),
      total_decisions_(0),
      shadow_decisions_(0),
      suggestion_decisions_(0),
      auto_decisions_(0),
      guardrail_triggered_high_risk_(0),
      guardrail_triggered_illegal_location_(0),
      guardrail_triggered_low_confidence_(0),
      total_reward_(0.0),
      total_confidence_(0.0),
      total_uncertainty_(0.0),
      model_updates_(0),
      shadow_matches_rule_(0),
      shadow_differs_from_rule_(0),
      shadow_guardrail_triggered_(0) {
}

BanditEngine::~BanditEngine() {
}

void BanditEngine::setMode(BanditMode mode) {
    mode_ = mode;
}

Action BanditEngine::selectAction(const ContextFeatures& context) {
    total_decisions_++;
    
    switch (mode_) {
        case BanditMode::SHADOW:
            shadow_decisions_++;
            break;
        case BanditMode::SUGGESTION:
            suggestion_decisions_++;
            break;
        case BanditMode::AUTO:
            auto_decisions_++;
            break;
    }
    
    return algorithm_->selectAction(context);
}

std::vector<RankedAction> BanditEngine::getTopKRankedActions(const ContextFeatures& context, int k) const {
    return algorithm_->getTopKRankedActions(context, k);
}

PolicyMixer::MixedDecision BanditEngine::decide(const ContextFeatures& context, int top_k) {
    total_decisions_++;
    
    auto decision = PolicyMixer::decide(context, *algorithm_, top_k);
    
    // 更新 Guardrail 统计
    if (decision.source == "GUARDRAIL") {
        if (decision.reason.find("HIGH_RISK") != std::string::npos) {
            guardrail_triggered_high_risk_++;
        } else if (decision.reason.find("ILLEGAL_LOCATION") != std::string::npos) {
            guardrail_triggered_illegal_location_++;
        } else if (decision.reason.find("LOW_CONFIDENCE") != std::string::npos) {
            guardrail_triggered_low_confidence_++;
        }
        
        if (mode_ == BanditMode::SHADOW) {
            shadow_guardrail_triggered_++;
        }
    }
    
    // 更新置信度和不确定性统计
    if (!decision.top_k_actions.empty()) {
        total_confidence_ += decision.top_k_actions[0].confidence;
        total_uncertainty_ += decision.top_k_actions[0].uncertainty;
    }
    
    // 更新模式统计
    switch (mode_) {
        case BanditMode::SHADOW:
            shadow_decisions_++;
            break;
        case BanditMode::SUGGESTION:
            suggestion_decisions_++;
            break;
        case BanditMode::AUTO:
            auto_decisions_++;
            break;
    }
    
    return decision;
}

GuardrailDecision BanditEngine::checkGuardrail(const ContextFeatures& context, double confidence) const {
    return GuardrailModule::checkGuardrail(context, confidence);
}

std::vector<std::pair<Action, double>> BanditEngine::getAllPredictions(const ContextFeatures& context) const {
    return algorithm_->getAllPredictions(context);
}

void BanditEngine::recordDecision(int decision_id, const ContextFeatures& context, const Action& action,
                                   const std::string& decision_source, double confidence, double uncertainty) {
    std::lock_guard<std::mutex> lock(mutex_);
    decision_cache_[decision_id] = {context, action, decision_source, confidence, uncertainty, false};
}

void BanditEngine::updateReward(int decision_id, double reward) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = decision_cache_.find(decision_id);
    if (it != decision_cache_.end()) {
        algorithm_->update(it->second.context, it->second.action, reward);
        it->second.reward_received = true;
        total_reward_ += reward;
        model_updates_++;
        
        if (decision_cache_.size() > 1000) {
            decision_cache_.erase(decision_cache_.begin(), std::next(decision_cache_.begin(), 500));
        }
    }
}

void BanditEngine::updateRewardV33(int decision_id, double reward, bool actually_missing,
                                    double inspect_ratio, double no_action_ratio) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = decision_cache_.find(decision_id);
    if (it != decision_cache_.end()) {
        algorithm_->updateV33(it->second.context, it->second.action, reward,
                              actually_missing, inspect_ratio, no_action_ratio);
        it->second.reward_received = true;
        total_reward_ += reward;
        model_updates_++;
        
        if (decision_cache_.size() > 1000) {
            decision_cache_.erase(decision_cache_.begin(), std::next(decision_cache_.begin(), 500));
        }
    }
}

void BanditEngine::addDelayedReward(int decision_id, double reward, int delay_hours) {
    std::lock_guard<std::mutex> lock(mutex_);
    delayed_rewards_.push_back({decision_id, reward, delay_hours, "pending"});
}

void BanditEngine::processDelayedRewards() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto now = std::chrono::system_clock::now();
    auto it = delayed_rewards_.begin();
    
    while (it != delayed_rewards_.end()) {
        updateReward(it->decision_id, it->reward);
        it->status = "processed";
        it = delayed_rewards_.erase(it);
    }
}

BanditEngine::Statistics BanditEngine::getStatistics() const {
    Statistics stats;
    stats.total_decisions = total_decisions_;
    stats.shadow_decisions = shadow_decisions_;
    stats.suggestion_decisions = suggestion_decisions_;
    stats.auto_decisions = auto_decisions_;
    stats.guardrail_triggered_high_risk = guardrail_triggered_high_risk_;
    stats.guardrail_triggered_illegal_location = guardrail_triggered_illegal_location_;
    stats.guardrail_triggered_low_confidence = guardrail_triggered_low_confidence_;
    stats.avg_reward = model_updates_ > 0 ? total_reward_ / model_updates_ : 0.0;
    stats.avg_confidence = total_decisions_ > 0 ? total_confidence_ / total_decisions_ : 0.0;
    stats.avg_uncertainty = total_decisions_ > 0 ? total_uncertainty_ / total_decisions_ : 0.0;
    stats.model_updates = model_updates_;
    return stats;
}

BanditEngine::ShadowStats BanditEngine::getShadowStats() const {
    ShadowStats stats;
    stats.shadow_matches_rule = shadow_matches_rule_;
    stats.shadow_differs_from_rule = shadow_differs_from_rule_;
    stats.shadow_guardrail_triggered = shadow_guardrail_triggered_;
    return stats;
}

void BanditEngine::saveModels(const std::string& path) {
    auto models = algorithm_->getModels();
    json j;
    
    for (const auto& arm : models) {
        json arm_json;
        arm_json["action"] = arm.action.toString();
        arm_json["threshold_adjustment"] = arm.action.threshold_adjustment;
        arm_json["confidence_boost"] = arm.action.confidence_boost;
        arm_json["sample_count"] = arm.sample_count;
        
        json a_matrix;
        for (int i = 0; i < arm.A.rows(); ++i) {
            json row;
            for (int j = 0; j < arm.A.cols(); ++j) {
                row.push_back(arm.A(i, j));
            }
            a_matrix.push_back(row);
        }
        arm_json["A"] = a_matrix;
        
        json b_vector;
        for (int i = 0; i < arm.b.size(); ++i) {
            b_vector.push_back(arm.b(i));
        }
        arm_json["b"] = b_vector;
        
        j.push_back(arm_json);
    }
    
    std::ofstream file(path);
    file << j.dump(4);
}

void BanditEngine::loadModels(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) return;
    
    json j;
    file >> j;
    
    std::vector<ArmModel> models;
    for (const auto& arm_json : j) {
        Action action;
        action.type = Action::fromString(arm_json["action"]);
        action.threshold_adjustment = arm_json["threshold_adjustment"];
        action.confidence_boost = arm_json["confidence_boost"];
        
        ArmModel arm(getFeatureDimension(), action);
        arm.sample_count = arm_json["sample_count"];
        
        auto a_matrix = arm_json["A"];
        for (int i = 0; i < a_matrix.size(); ++i) {
            for (int j = 0; j < a_matrix[i].size(); ++j) {
                arm.A(i, j) = a_matrix[i][j];
            }
        }
        
        auto b_vector = arm_json["b"];
        for (int i = 0; i < b_vector.size(); ++i) {
            arm.b(i) = b_vector[i];
        }
        
        models.push_back(arm);
    }
    
    algorithm_->loadModels(models);
}

void BanditEngine::reset() {
    algorithm_ = std::make_unique<LinUCBAlgorithm>(getFeatureDimension(), 1.0);
    total_decisions_ = 0;
    shadow_decisions_ = 0;
    suggestion_decisions_ = 0;
    auto_decisions_ = 0;
    guardrail_triggered_high_risk_ = 0;
    guardrail_triggered_illegal_location_ = 0;
    guardrail_triggered_low_confidence_ = 0;
    total_reward_ = 0.0;
    total_confidence_ = 0.0;
    total_uncertainty_ = 0.0;
    model_updates_ = 0;
    shadow_matches_rule_ = 0;
    shadow_differs_from_rule_ = 0;
    shadow_guardrail_triggered_ = 0;
    decision_cache_.clear();
    delayed_rewards_.clear();
}

} // namespace bandit
