#pragma once

#include <cstdint>

enum TankType
{
    TANK_SHERMAN,
    TANK_PANZER
};

//players are handed ids starting at 1, and each new player takes the next model in the rotation:
//player 1 is a Sherman, player 2 a Panzer IV, player 3 a Sherman again, and so on
inline TankType GetTankTypeForPlayerId(uint32_t inPlayerId)
{
    const uint32_t kTankTypeCount = 2;

    //ids start at 1, so shift down before picking. an id of 0 means we haven't been told who
    //this tank belongs to yet, and it gets the first model until the real id replicates in
    uint32_t index = (inPlayerId > 0 ? inPlayerId - 1 : 0) % kTankTypeCount;

    return (index == 0) ? TANK_SHERMAN : TANK_PANZER;
}

inline const char* GetTankBodyTextureName(TankType inTankType)
{
    return (inTankType == TANK_PANZER) ? "panzer" : "sherman";
}

inline const char* GetTankTurretTextureName(TankType inTankType)
{
    return (inTankType == TANK_PANZER) ? "panzer_turret" : "sherman_turret";
}
