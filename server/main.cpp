#include <grpcpp/grpcpp.h>
#include "rpc/asset_service.h"
#include "rpc/monitoring_service.h"
#include "rpc/admin_service.h"
#include "rpc/bi_service.h"
#include "rpc/trajectory_service.h"
#include "core/logger.h"
#include <iostream>
#include <csignal>

namespace {
    std::unique_ptr<grpc::Server> g_server;
    std::mutex g_server_mutex;
}

void signalHandler(int signal) {
    if (signal == SIGINT || signal == SIGTERM) {
        std::lock_guard<std::mutex> lock(g_server_mutex);
        if (g_server) {
            std::cout << "\nShutting down server..." << std::endl;
            spdlog::info("Received shutdown signal, stopping server...");
            g_server->Shutdown();
        }
    }
}

int main() {
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    Logger::init("rfid_server");
    spdlog::info("Starting RFID Server v3.3.1...");

    std::string server_address("0.0.0.0:50051");
    AssetServiceImpl assetService;
    MonitoringServiceImpl monitoringService;
    AdminServiceImpl adminService;
    rpc::BIReportServiceImpl biService;
    rpc::TrajectoryServiceImpl trajectoryService;

    grpc::ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&assetService);
    builder.RegisterService(&monitoringService);
    builder.RegisterService(&adminService);
    builder.RegisterService(&biService);
    builder.RegisterService(&trajectoryService);

    {
        std::lock_guard<std::mutex> lock(g_server_mutex);
        g_server = builder.BuildAndStart();
    }

    std::cout << "Server listening on " << server_address << std::endl;
    spdlog::info("Server listening on {}", server_address);

    g_server->Wait();

    Logger::shutdown();
    return 0;
}
