#include "asset_code_generator.h"
#include "../core/logger.h"

AssetCodeGenerator& AssetCodeGenerator::instance() {
    static AssetCodeGenerator instance;
    return instance;
}

AssetCodeGenerator::AssetCodeGenerator()
    : counter_(1), baseYear_(2026), lastGeneratedCode_(0) {
    spdlog::info("AssetCodeGenerator initialized with base year {}", baseYear_);
}

std::string AssetCodeGenerator::generate(const std::string& prefix) {
    int code = ++lastGeneratedCode_;
    std::string assetCode = prefix + "-" + std::to_string(baseYear_) + "-" + std::to_string(code);
    spdlog::debug("Generated asset code: {}", assetCode);
    return assetCode;
}

void AssetCodeGenerator::setBaseYear(int year) {
    baseYear_ = year;
    spdlog::info("AssetCodeGenerator base year set to {}", year);
}

void AssetCodeGenerator::setStartNumber(int num) {
    lastGeneratedCode_ = num;
    spdlog::info("AssetCodeGenerator start number set to {}", num);
}