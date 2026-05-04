#pragma once

#include <vector>
#include <optional>
#include "models/decision.h"
#include "models/decision_snapshot.h"

class DecisionRepository {
public:
    static DecisionRepository& instance();

    void insert(const DecisionRecord& record);

    int insertWithId(const DecisionRecord& record);

    std::vector<DecisionRecord> getRecent(int limit);

    void markExecuted(int assetId, bool executed, bool ignored, const std::string& operatorName);

    double getAdoptionRate(int hours = 24);

    void insertSnapshot(const DecisionSnapshot& snapshot);

    std::optional<DecisionSnapshot> getSnapshotByDecisionId(int decisionId);

private:
    DecisionRepository() = default;
};
