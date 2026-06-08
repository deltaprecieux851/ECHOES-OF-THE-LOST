#include "game/EchoSystem.h"

#include "core/Logger.h"

namespace echoes::game {

namespace {

constexpr float kResonanceDrainPerSecond = 12.0f;
constexpr float kResonanceRegenPerSecond = 6.0f;

}  // namespace

void EchoSystem::Update(float deltaTime) {
    const bool anyActive = temporalSlowActive_ || spectralVisionActive_ || memoryInvocationActive_;

    if (anyActive) {
        resonance_ -= kResonanceDrainPerSecond * deltaTime;
        if (resonance_ <= 0.0f) {
            resonance_ = 0.0f;
            temporalSlowActive_ = false;
            spectralVisionActive_ = false;
            memoryInvocationActive_ = false;
            core::Logger::Log(core::LogLevel::Warning, "Psychoresonance depleted.");
        }
    } else if (resonance_ < 100.0f) {
        resonance_ += kResonanceRegenPerSecond * deltaTime;
        if (resonance_ > 100.0f) {
            resonance_ = 100.0f;
        }
    }
}

bool EchoSystem::ActivatePower(EchoPower power) {
    if (resonance_ <= 0.0f) {
        return false;
    }

    switch (power) {
        case EchoPower::TemporalSlow:
            temporalSlowActive_ = true;
            core::Logger::Log(core::LogLevel::Info, "Echo power: temporal slow activated.");
            return true;
        case EchoPower::SpectralVision:
            spectralVisionActive_ = true;
            core::Logger::Log(core::LogLevel::Info, "Echo power: spectral vision activated.");
            return true;
        case EchoPower::MemoryInvocation:
            memoryInvocationActive_ = true;
            core::Logger::Log(core::LogLevel::Info, "Echo power: memory invocation activated.");
            return true;
    }
    return false;
}

void EchoSystem::DeactivatePower(EchoPower power) {
    switch (power) {
        case EchoPower::TemporalSlow:      temporalSlowActive_ = false; break;
        case EchoPower::SpectralVision:    spectralVisionActive_ = false; break;
        case EchoPower::MemoryInvocation:  memoryInvocationActive_ = false; break;
    }
}

bool EchoSystem::IsPowerActive(EchoPower power) const {
    switch (power) {
        case EchoPower::TemporalSlow:      return temporalSlowActive_;
        case EchoPower::SpectralVision:    return spectralVisionActive_;
        case EchoPower::MemoryInvocation:  return memoryInvocationActive_;
    }
    return false;
}

}  // namespace echoes::game
