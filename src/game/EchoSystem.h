#pragma once

namespace echoes::game {

enum class EchoPower {
    TemporalSlow,
    SpectralVision,
    MemoryInvocation
};

class EchoSystem {
public:
    void Update(float deltaTime);

    bool ActivatePower(EchoPower power);
    void DeactivatePower(EchoPower power);

    float GetResonance() const { return resonance_; }
    bool IsPowerActive(EchoPower power) const;

private:
    float resonance_{100.0f};
    bool temporalSlowActive_{false};
    bool spectralVisionActive_{false};
    bool memoryInvocationActive_{false};
};

}  // namespace echoes::game
