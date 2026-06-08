#include "networking/NetworkStub.h"

#include "core/Logger.h"

namespace echoes::networking {

bool NetworkManager::HostSession(const SessionConfig& config) {
    config_ = config;
    role_ = NetworkRole::Host;
    core::Logger::Log(core::LogLevel::Info, "Network stub: hosting co-op session (not yet implemented).");
    return true;
}

bool NetworkManager::JoinSession(const std::string& hostId) {
    role_ = NetworkRole::Client;
    core::Logger::Log(core::LogLevel::Info, "Network stub: joining session " + hostId);
    return true;
}

void NetworkManager::Disconnect() {
    role_ = NetworkRole::Offline;
}

void NetworkManager::Update(float /*deltaTime*/) {}

}  // namespace echoes::networking
