#pragma once

enum class TileType
{
    Empty,
    Wall,
    Exit,
    Danger,
    Trap
};

struct Tile
{
    TileType type = TileType::Empty;
    bool active = true;
};
