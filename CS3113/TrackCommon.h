#ifndef TRACKCOMMON_H
#define TRACKCOMMON_H

#include <vector>

constexpr int TRACK_WIDTH  = 40;
constexpr int TRACK_HEIGHT = 30;

struct Corner {
    std::vector<std::pair<int,int>> tiles;   // (row, col) pairs for the corner
    float angle;
};

#endif
