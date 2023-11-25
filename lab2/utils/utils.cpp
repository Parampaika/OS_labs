#include "utils.h"
#include <random>

extern int GetRand(int limit)
{
    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_int_distribution<int> dist(1, limit);
    return dist(mt);
}

