// Maze.h
#pragma once
#include <vector>
#include <string>

enum class TileType { Wall, Floor, Start, Exit, Door };

struct Cell {
    TileType type = TileType::Wall;
    int doorIndex = -1; // 0..4 para D1..D5
};

struct Maze {
    int w=0, h=0;
    std::vector<Cell> cells;
    int sr=0, sc=0;  // start
    int er=0, ec=0;  // exit

    const Cell& at(int r, int c) const { return cells[r*w + c]; }
    Cell& at(int r, int c) { return cells[r*w + c]; }
};
