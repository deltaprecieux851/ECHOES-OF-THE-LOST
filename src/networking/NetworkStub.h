#pragma once

#include <cstdint>
#include <string>

namespace echoes::networking {

// Placeholder for future Steam NetSockets P2P replication layer.
// Architecture anticipates ECS-based state replication for co-op (max 3 players).

enum class NetworkRole {
    Offline,
    Host,
    Client
};

struct SessionConfig {
    std::string relayAddress;
    std::uint16_t port{27015};
    std::uint8_t maxPlayers{3};
};

class NetworkManager {
public:
    bool HostSession(const SessionConfig& config);
    bool JoinSession(const std::string& hostId);
    void Disconnect();
    void Update(float deltaTime);

    NetworkRole GetRole() const { return role_; }
    bool IsConnected() const { return role_ != NetworkRole::Offline; }

private:
    NetworkRole role_{NetworkRole::Offline};
    SessionConfig config_;
};

}  // namespace echoes::networking
