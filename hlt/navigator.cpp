#include "navigator.h"
#include "shipStatus.hpp"

#include <iostream>
#include <algorithm>
#include <cassert>

using namespace std;
using namespace hlt;

Navigator::Navigator(shared_ptr<GameMap>& gameMap, shared_ptr<Player>& me,
                     unordered_map<EntityId, ShipStatus>& shipStatus, mt19937& rng) :
    gameMap_(gameMap), me_(me), shipStatus_(shipStatus), usedPosiitons_(), rng_(rng) {
        maxHalitePotential_ = calculateMaxHalitePotential();
        log::log("Max Halite Potential " + std::to_string(maxHalitePotential_));
        halitePotentialNavigateThreshold_ = calculateHalitePotentialNavigateThreshold(maxHalitePotential_);
        Tunables tunables;
        lowestHaliteToCollect_ = maxHalitePotential_ * tunables.lookUpTunable("colPrecent");
        log::log("Low Halite Collection " + std::to_string(lowestHaliteToCollect_));
    }

double Navigator::calculateMaxHalitePotential() {
    Tunables tunables;
    Position shipyard = me_->shipyard->position;
    int size = gameMap_->width;
    double maxPotential = 1;
    for(int x = 0; x < size; x++) {
        for(int y = 0; y < size; y++) {
            Position pos = Position(x, y);
            int dist = gameMap_->calculate_distance(shipyard, pos);
            int halite = gameMap_->at(pos)->halite;
            // look at the surrounding position, if the halite at this point is absurdly high,
            // don't account it for navigation.
            array<Position, 4> surroundingPositions = pos.get_surrounding_cardinals();
            int surroundingHalite = 0;
            for (Position surroundingPos : surroundingPositions) {
                surroundingHalite += gameMap_->at(surroundingPos)->halite;
            }
            surroundingHalite /= 4;
            if (halite > surroundingHalite * 3) {
                // this is absurd, probably caused by collision
                halite = surroundingHalite;
            }
            double potential = (halite / (2 * dist * tunables.lookUpTunable("hltCorr0") + tunables.lookUpTunable("hltCorr1")));
            if (potential > maxPotential) {
                //log::log(std::to_string(potential));
                //log::log("halite " + std::to_string(halite));
                //log::log("dist " + std::to_string(dist));
                maxPotential = potential;
            }
        }
    }
    return maxPotential;
}

vector<Direction> Navigator::createShip(shared_ptr<Ship> ship) {
    vector<Direction> nextDirs = explore(ship);
    vector<Direction> wiggleDirs = wiggleDirectionsMostHalite(ship);
    nextDirs.insert(nextDirs.end(), wiggleDirs.begin(), wiggleDirs.end());
    return nextDirs;
}

vector<Direction> Navigator::explore(shared_ptr<Ship> ship) {
    vector<Position> homePositions;
    // get all the home positions :

    homePositions.push_back(me_->shipyard->position);
    for (auto dropoffPair : me_->dropoffs) {
        homePositions.push_back(dropoffPair.second->position);
    }

    Position shipPos = ship->position;

    Comparator * comparator = new Comparator(gameMap_, ship, homePositions, shipPos, halitePotentialNavigateThreshold_);

    int shipLookAhead = Tunables::SHIP_LOOKS_AHEAD;
    vector<Position> posList = getSurroundingPositions(shipPos, shipLookAhead);

    while(comparator->noValidPosition(posList)){
        shipLookAhead *= 2;

        posList = getSurroundingPositions(shipPos, shipLookAhead);

        if (shipLookAhead >= gameMap_->width) {
                return vector<Direction>();
        }
    }

    vector<Position> filteredPosList;
    for (Position pos : posList) {
        if (!usedPosiitons_.count(pos)) {
            filteredPosList.push_back(pos);
        }
    }

    //Retourne le plus grand élément du vecteur en comparant tous ces éléments
    Position maxPos = *comparator->maxElement(posList.begin(), posList.end());

    usedPosiitons_.insert(maxPos);

    vector<Direction> targetDirs = gameMap_->get_unsafe_moves(shipPos, maxPos);

    //Expression lambda en c++ permettant
    std::sort(targetDirs.begin(), targetDirs.end(), [&](const Direction& dir1, const Direction& dir2)
    {
        Position posA = gameMap_->destination_position(ship->position, dir1);
        Position posB = gameMap_->destination_position(ship->position, dir2);
        return gameMap_->at(posA)->halite > gameMap_->at(posB)->halite;
    });

    // add some wiggle
    vector<Direction> wiggleDirs = wiggleDirectionsMostHalite(ship);
    targetDirs.push_back(wiggleDirs[0]);
    return targetDirs;
}

double Navigator::calculateHalitePotentialNavigateThreshold(double maxHalitePotential) {
    Tunables tunables;
    return maxHalitePotential * tunables.lookUpTunable("navPercent");
}

vector<Position> Navigator::getSurroundingPositions(Position middlePos, int lookAhead) {
    vector<Position> posList;
    for(int xDiff = -lookAhead; xDiff <= lookAhead; xDiff++) {
        for(int yDiff = -lookAhead; yDiff <= lookAhead; yDiff++) {
            Position newPos = Position(middlePos.x + xDiff, middlePos.y + yDiff);
            gameMap_->normalize(newPos);
            posList.push_back(newPos);
        }
    }
    return posList;
}

vector<Direction> Navigator::wiggleDirectionsMostHalite(shared_ptr<Ship> ship) {
    vector<Direction> shuffledDirs = vector<Direction>(ALL_CARDINALS.begin(), ALL_CARDINALS.end());
    shuffle(shuffledDirs.begin(), shuffledDirs.end(), rng_);

    sort(shuffledDirs.begin(), shuffledDirs.end(),  [&](const Direction& dir1, const Direction& dir2)
    {
        Position posA = gameMap_->destination_position(ship->position, dir1);
        Position posB = gameMap_->destination_position(ship->position, dir2);
        return gameMap_->at(posA)->halite > gameMap_->at(posB)->halite;
    });

    return shuffledDirs;
}

// Exploring ship nearby and potentially do not know what to do
bool Navigator::confusedExploringShipNearby(shared_ptr<Ship> ship) {
    for (Direction direction : ALL_CARDINALS) {
        Position nearPos = gameMap_->destination_position(ship->position, direction);
        if (gameMap_->at(nearPos)->is_occupied()) {
            shared_ptr<Ship> nearShip = gameMap_->at(nearPos)->ship;
            // if it is our ship
            if (nearShip->owner == me_->id) {
                assert(shipStatus_.count(nearShip->id));
                // if the nearby ship is either explore or new
                // and if its position has less halite than this ship's
                // position, then return true
                if (shipStatus_[nearShip->id] == ShipStatus::EXPLORE) {
                    if(gameMap_->at(ship->position)->halite > gameMap_->at(nearShip->position)->halite) {
                        return true;
                    }
                }

                if(shipStatus_[nearShip->id] == ShipStatus::NEW) {
                    if(gameMap_->at(ship->position)->halite > gameMap_->at(nearShip->position)->halite) {
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

vector<Direction> Navigator::collect(shared_ptr<Ship> ship) {
    // if there is another exploring ship close by, create a space for it.
    if (confusedExploringShipNearby(ship)) {
        vector<Direction> outDirs;
        vector<Direction> targetDirs = wiggleDirectionsMostHalite(ship);
        for (Direction direction : targetDirs) {
            Position targetPos = gameMap_->destination_position(ship->position, direction);
            if (gameMap_->at(targetPos)->halite > gameMap_->at(ship)->halite) {
                outDirs.push_back(direction);
            }
        }
        return outDirs;
    }
    return { Direction::STILL };
}

vector<Direction> Navigator::dropoffHalite(shared_ptr<Ship> ship) {
    // go to the closest shipyard or dropoffs.
    vector<Position> targetPositions;
    targetPositions.push_back(me_->shipyard->position);
    for (auto dropoffpair : me_->dropoffs) {
        Position dropoffPos = dropoffpair.second->position;
        targetPositions.push_back(dropoffPos);
    }

    Position targetPosition = targetPositions[0];
    for (Position currentPosition : targetPositions) {
        if (gameMap_->calculate_distance(ship->position, currentPosition) < gameMap_->calculate_distance(ship->position, targetPosition)) {
            targetPosition = currentPosition;
        }
    }

    vector<Direction> nextDirs = gameMap_->get_unsafe_moves(ship->position, targetPosition);

    sort(nextDirs.begin(), nextDirs.end(),  [&](const Direction& dir1, const Direction& dir2)
    {
        Position posA = gameMap_->destination_position(ship->position, dir1);
        Position posB = gameMap_->destination_position(ship->position, dir2);
        return gameMap_->at(posA)->halite > gameMap_->at(posB)->halite;
    });

    vector<Direction> wiggleDirs = wiggleDirectionsMostHalite(ship);
    sort(wiggleDirs.begin(), wiggleDirs.end(), [&](const Direction& dir1, const Direction& dir2)
    {
        Position posA = gameMap_->destination_position(ship->position, dir1);
        Position posB = gameMap_->destination_position(ship->position, dir2);
        return gameMap_->at(posA)->halite > gameMap_->at(posB)->halite;
    });
    nextDirs.push_back(wiggleDirs[0]);
    nextDirs.push_back(wiggleDirs[1]);
    return nextDirs;
}
