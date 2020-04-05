#ifndef COMPARATOR_H
#define COMPARATOR_H

#include "position.hpp"
#include "types.hpp"
#include "game_map.hpp"
#include "ship.hpp"
#include "dropoff.hpp"
#include "game.hpp"
#include "constants.hpp"

#include "tunables.h"

#include <vector>
#include <random>
#include <unordered_map>

using namespace std;
using namespace hlt;

class Comparator
{
public:
    Comparator(shared_ptr<GameMap> gameMap, shared_ptr<Ship> ship, vector<Position> centerPositions, Position & shipPos, double halitePotentialNavigateThreshold );
    ~Comparator();

    bool pos1HasMoreHaliteThanPos2(const Position& pos1, const Position& pos2);

    bool dir1HasMoreHaliteThanDir2(const Direction& dir1, const Direction& dir2);

    bool bestPos (const Position& pos1, const Position& pos2) const;

    double evaluatePosition(Position pos) const;

    bool noValidPosition(vector<Position> posList) const;

    template<class ForwardIt>
    ForwardIt maxElement(ForwardIt first, ForwardIt last)
    {
        if (first == last) return last;

        ForwardIt largest = first;
        ++first;
        for (; first != last; ++first) {
            if (bestPos(*largest, *first)) {
                largest = first;
            }
        }
        return largest;
    }


private:
    shared_ptr<GameMap> gameMap_;
    shared_ptr<Ship> ship_;
    vector<Position> centerPositions_;
    Position shipPos_;
    double halitePotentialNavigateThreshold_;
};

#endif // COMPARATOR_H
