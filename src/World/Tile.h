#pragma once

enum class TileType
{
    Empty,
    Wall,
    Exit,
    Danger,
    Trap,
    Curse,
    Defuse
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
};
