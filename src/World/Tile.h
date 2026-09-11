#pragma once

enum class TileType
{
    Empty,
    Wall,
    Exit,
    Danger,
    Trap,
    Curse,
    Defuse,
    Ice,
    Crumbling,
    Pit,
    Portal,
    Key,
    Gate,
    Bomb,
    Shield,
    TimeFreeze,
    Coin
};

enum class DebuffType
{
    None,
    ReverseControls,
    TeleportSpawn,
    ReviseMap,
    TimePenalty
};

struct Tile
{
    TileType type = TileType::Empty;
    bool active = true;
    DebuffType debuff = DebuffType::None;
    int portalTargetX = -1;
    int portalTargetY = -1;
    int fuse = 0;
};
