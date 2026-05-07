#include "decision_panel_model.h"

DecisionPanelModel::DecisionPanelModel(QObject* parent)
    : QObject(parent),
      confidence_(0.0),
      missing_risk_(0.0),
      abnormal_risk_(0.0),
      idle_risk_(0.0),
      visible_(false),
      current_asset_id_(0) {
}

QString DecisionPanelModel::assetName() const {
    return asset_name_;
}

QString DecisionPanelModel::action() const {
    return action_;
}

double DecisionPanelModel::confidence() const {
    return confidence_;
}

double DecisionPanelModel::missingRisk() const {
    return missing_risk_;
}

double DecisionPanelModel::abnormalRisk() const {
    return abnormal_risk_;
}

double DecisionPanelModel::idleRisk() const {
    return idle_risk_;
}

QString DecisionPanelModel::reason() const {
    return reason_;
}

QVariantList DecisionPanelModel::topKActions() const {
    return top_k_actions_;
}

bool DecisionPanelModel::visible() const {
    return visible_;
}

void DecisionPanelModel::show(uint64_t assetId, double x, double y) {
    current_asset_id_ = assetId;
    visible_ = true;
    emit visibleChanged();
    
    Q_UNUSED(x);
    Q_UNUSED(y);
}

void DecisionPanelModel::hide() {
    visible_ = false;
    emit visibleChanged();
}

void DecisionPanelModel::reportFeedback(const QString& action) {
    emit feedbackReported(asset_name_, action);
}

void DecisionPanelModel::updateOverlayData(const DecisionOverlay& overlay) {
    asset_name_ = QString("Asset-%1").arg(overlay.asset_id);
    action_ = actionToString(overlay.action);
    confidence_ = overlay.confidence;
    missing_risk_ = overlay.missing_risk;
    abnormal_risk_ = overlay.abnormal_risk;
    idle_risk_ = overlay.inactivity_risk;
    reason_ = QString::fromUtf8(overlay.reason);
    
    emit assetNameChanged();
    emit actionChanged();
    emit confidenceChanged();
    emit missingRiskChanged();
    emit abnormalRiskChanged();
    emit idleRiskChanged();
    emit reasonChanged();
}

void DecisionPanelModel::setTopKActions(const std::vector<std::pair<std::string, double>>& actions) {
    top_k_actions_.clear();
    
    for (const auto& pair : actions) {
        QVariantMap map;
        map["action"] = QString::fromStdString(pair.first);
        map["score"] = pair.second;
        top_k_actions_.append(map);
    }
    
    emit topKActionsChanged();
}

QString DecisionPanelModel::actionToString(uint32_t action) const {
    switch (action) {
        case 1: return "INSPECT";
        case 2: return "ALERT";
        case 3: return "REALLOCATE";
        default: return "NO_ACTION";
    }
}