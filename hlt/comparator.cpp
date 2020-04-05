#include "comparator.h"

using namespace std;

Comparator::Comparator(shared_ptr<GameMap> gameMap, shared_ptr<Ship> ship, vector<Position> centerPositions, Position & shipPos, double halitePotentialNavigateThreshold)
    :gameMap_(gameMap),
      ship_(ship)
{
    centerPositions_ = centerPositions;
    shipPos_ = shipPos;
    halitePotentialNavigateThreshold_ = halitePotentialNavigateThreshold;
}

Comparator::~Comparator()
{

}

bool Comparator::pos1HasMoreHaliteThanPos2(const Position& pos1, const Position& pos2)
{
    return gameMap_->at(pos1)->halite > gameMap_->at(pos2)->halite;
}

bool Comparator::dir1HasMoreHaliteThanDir2(const Direction& dir1, const Direction& dir2)
{
    Position posA = gameMap_->destination_position(ship_->position, dir1);

    Position posB = gameMap_->destination_position(ship_->position, dir2);

    return gameMap_->at(posA)->halite > gameMap_->at(posB)->halite;
}

bool Comparator::bestPos( const Position& pos1, const Position& pos2) const
{
    double valA = evaluatePosition(pos1);
    double valB = evaluatePosition(pos2);
    return valA < valB;
}

double Comparator::evaluatePosition(Position pos) const {
    Tunables tunables;

    int minDist = 1 << 20;

    for (Position centerPos : centerPositions_) {
        int dist = gameMap_->calculate_distance(centerPos, pos);
        if (dist < minDist) {
            minDist = dist;
        }
    }

    int distHome = minDist;

    int distCollect = gameMap_->calculate_distance(shipPos_, pos);

    int dist = distHome + distCollect;

    int halite = std::min(gameMap_->at(pos)->halite, 800);

    double potential = halite / (dist * tunables.lookUpTunable("hltCorr0") + tunables.lookUpTunable("hltCorr1"));

    return max(0.0, potential - halitePotentialNavigateThreshold_);
}

bool Comparator::noValidPosition(vector<Position> posList) const
{
    for (Position pos : posList) {
        if (evaluatePosition(pos) > 0.1)
            return false;
    }
    return true;
}
