#include <iostream>
#include <vector>
#include <string>
#include <random>
#include <chrono>
#include <thread>
#include <atomic>
#include <cstdlib>
#include <ctime>
#include <iomanip>

struct LoadTestConfig {
    int targetEPCPerSecond = 1000;
    int durationSeconds = 60;
    int readerCount = 10;
    int batchSize = 50;
    std::string serverHost = "localhost";
    int serverPort = 50051;
};

struct LoadTestResult {
    int64_t totalEPCSent = 0;
    int64_t totalEPCReceived = 0;
    double actualEPCPerSecond = 0.0;
    double successRate = 0.0;
    double avgLatencyMs = 0.0;
    double maxLatencyMs = 0.0;
    double minLatencyMs = 0.0;
};

class LoadTester {
public:
    LoadTester(const LoadTestConfig& config) : config_(config) {
        std::srand(static_cast<unsigned>(std::time(nullptr)));
    }

    LoadTestResult run() {
        LoadTestResult result;
        auto startTime = std::chrono::steady_clock::now();

        std::atomic<int64_t> epcSent{0};
        std::atomic<int64_t> epcReceived{0};
        std::atomic<int64_t> errorCount{0};
        std::vector<double> latencies;

        std::mutex latenciesMutex;

        int intervalMs = (config_.batchSize * 1000) / config_.targetEPCPerSecond;

        std::vector<std::thread> readerThreads;

        for (int r = 0; r < config_.readerCount; ++r) {
            readerThreads.emplace_back([&, r]() {
                std::string readerId = "LOAD-R-" + std::to_string(r);

                while (true) {
                    auto batchStart = std::chrono::steady_clock::now();

                    std::vector<std::string> batch;
                    for (int i = 0; i < config_.batchSize; ++i) {
                        batch.push_back(generateEPC());
                    }

                    auto sendStart = std::chrono::steady_clock::now();

                    bool success = sendBatch(readerId, batch);

                    auto sendEnd = std::chrono::steady_clock::now();
                    double latencyMs = std::chrono::duration<double, std::milli>(sendEnd - sendStart).count();

                    {
                        std::lock_guard<std::mutex> lock(latenciesMutex);
                        latencies.push_back(latencyMs);
                    }

                    if (success) {
                        epcSent.fetch_add(config_.batchSize);
                    } else {
                        errorCount.fetch_add(config_.batchSize);
                    }

                    auto batchEnd = std::chrono::steady_clock::now();
                    auto batchDuration = std::chrono::duration_cast<std::chrono::milliseconds>(batchEnd - batchStart).count();

                    if (batchDuration < intervalMs) {
                        std::this_thread::sleep_for(std::chrono::milliseconds(intervalMs - batchDuration));
                    }

                    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
                        std::chrono::steady_clock::now() - startTime).count();
                    if (elapsed >= config_.durationSeconds) {
                        break;
                    }
                }
            });
        }

        for (auto& t : readerThreads) {
            t.join();
        }

        auto endTime = std::chrono::steady_clock::now();
        double totalSeconds = std::chrono::duration<double>(endTime - startTime).count();

        result.totalEPCSent = epcSent.load();
        result.totalEPCReceived = epcReceived.load();
        result.actualEPCPerSecond = result.totalEPCSent / totalSeconds;

        int64_t total = result.totalEPCSent + errorCount.load();
        result.successRate = total > 0 ? static_cast<double>(result.totalEPCSent) / total : 0.0;

        if (!latencies.empty()) {
            double sum = 0.0;
            double max = latencies[0];
            double min = latencies[0];

            for (double l : latencies) {
                sum += l;
                if (l > max) max = l;
                if (l < min) min = l;
            }

            result.avgLatencyMs = sum / latencies.size();
            result.maxLatencyMs = max;
            result.minLatencyMs = min;
        }

        return result;
    }

private:
    std::string generateEPC() {
        std::string epc = "EPC-";
        for (int i = 0; i < 4; ++i) {
            epc += std::to_string(std::rand() % 65536);
            if (i < 3) epc += "-";
        }
        return epc;
    }

    bool sendBatch(const std::string& readerId, const std::vector<std::string>& epcs) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        return true;
    }

    LoadTestConfig config_;
};

void printResult(const LoadTestResult& result) {
    std::cout << "\n========================================\n";
    std::cout << "          LOAD TEST RESULT\n";
    std::cout << "========================================\n";
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "Total EPC Sent:       " << result.totalEPCSent << "\n";
    std::cout << "Actual EPC/sec:       " << result.actualEPCPerSecond << "\n";
    std::cout << "Success Rate:         " << (result.successRate * 100) << "%\n";
    std::cout << "Avg Latency:          " << result.avgLatencyMs << " ms\n";
    std::cout << "Max Latency:          " << result.maxLatencyMs << " ms\n";
    std::cout << "Min Latency:          " << result.minLatencyMs << " ms\n";
    std::cout << "========================================\n";
}

void printUsage(const char* programName) {
    std::cout << "Usage: " << programName << " [options]\n";
    std::cout << "Options:\n";
    std::cout << "  -r <count>    Reader count (default: 10)\n";
    std::cout << "  -e <rate>    Target EPC per second (default: 1000)\n";
    std::cout << "  -d <seconds> Test duration in seconds (default: 60)\n";
    std::cout << "  -b <size>    Batch size per request (default: 50)\n";
    std::cout << "  -h <host>    Server host (default: localhost)\n";
    std::cout << "  -p <port>    Server port (default: 50051)\n";
    std::cout << "\nExample:\n";
    std::cout << "  " << programName << " -r 10 -e 5000 -d 300\n";
    std::cout << "  (10 readers, 5000 EPC/s, 5 minutes)\n";
}

int main(int argc, char* argv[]) {
    LoadTestConfig config;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "-r" && i + 1 < argc) {
            config.readerCount = std::atoi(argv[++i]);
        } else if (arg == "-e" && i + 1 < argc) {
            config.targetEPCPerSecond = std::atoi(argv[++i]);
        } else if (arg == "-d" && i + 1 < argc) {
            config.durationSeconds = std::atoi(argv[++i]);
        } else if (arg == "-b" && i + 1 < argc) {
            config.batchSize = std::atoi(argv[++i]);
        } else if (arg == "-h" && i + 1 < argc) {
            config.serverHost = argv[++i];
        } else if (arg == "-p" && i + 1 < argc) {
            config.serverPort = std::atoi(argv[++i]);
        } else if (arg == "--help" || arg == "-h") {
            printUsage(argv[0]);
            return 0;
        }
    }

    std::cout << "========================================\n";
    std::cout << "     RFID SYSTEM LOAD TEST v1.0\n";
    std::cout << "========================================\n";
    std::cout << "Configuration:\n";
    std::cout << "  Target EPC/sec:     " << config.targetEPCPerSecond << "\n";
    std::cout << "  Duration:           " << config.durationSeconds << " seconds\n";
    std::cout << "  Reader count:       " << config.readerCount << "\n";
    std::cout << "  Batch size:         " << config.batchSize << "\n";
    std::cout << "  Server:             " << config.serverHost << ":" << config.serverPort << "\n";
    std::cout << "========================================\n";

    LoadTester tester(config);
    LoadTestResult result = tester.run();

    printResult(result);

    return 0;
}