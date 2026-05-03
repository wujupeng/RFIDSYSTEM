#pragma once
#include <string>
#include <atomic>

class AssetCodeGenerator {
public:
    static AssetCodeGenerator& instance();

    std::string generate(const std::string& prefix = "IT-SZ");

    void setBaseYear(int year);
    void setStartNumber(int num);

private:
    AssetCodeGenerator();
    AssetCodeGenerator(const AssetCodeGenerator&) = delete;
    AssetCodeGenerator& operator=(const AssetCodeGenerator&) = delete;

    std::atomic<int> counter_;
    int baseYear_;
    std::atomic<int> lastGeneratedCode_;
};