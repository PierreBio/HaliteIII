#include "strategymap.h"

#include <queue>
#include <stack>
#include <algorithm>
#include <random>

using namespace std;
using namespace hlt;

StrategyMap::StrategyMap(shared_ptr<GameMap>& gameMap, shared_ptr<Player>& me, int nPlayers, mt19937& rng,
    bool collisionCenterOkay) :
rng_(rng) {
    me_ = me;
    nPlayers_ = nPlayers;
    gameMap_ = gameMap;
    shipsComingtoPos_ = {};
    shipDirectionQueue_ = {};
    allConflicts_ = {};
    shouldMakeShip_ = false;
    collisionCenterOkay_ = collisionCenterOkay;
    vector<Command> command_queue_ = {};
}

void StrategyMap::createDropoff(shared_ptr<Ship> ship) {
    command_queue_.push_back(ship->make_dropoff());
}

void StrategyMap::addIntent(shared_ptr<Ship> ship, vector<Direction> preferredDirs, bool ignoreOpponentFlag) {
    //log::log("Add intent: ship " + to_string(ship->id));
    shipIgnoresOpponent_[ship->position] = ignoreOpponentFlag;
    if (!gameMap_->can_move(ship)) {
        shipDirectionQueue_[ship->position].push(Direction::STILL);
        shipsComingtoPos_[ship->position].push_back(ship);
        return;
    }
    Position currentPos = ship->position;
    for (Direction dir : preferredDirs) {
        shipDirectionQueue_[ship->position].push(dir);
    }
    if (preferredDirs.empty()) {
        //log::log("Preferred direction should not be empty!");
        preferredDirs.push_back(Direction::STILL);
    }
    Direction mostPreferredDir = preferredDirs[0];
    Position newPos = gameMap_->destination_position(currentPos, mostPreferredDir);
    shipsComingtoPos_[newPos].push_back(ship);
}

void StrategyMap::createShip() {
    shouldMakeShip_ = true;
}


vector<Command> StrategyMap::processOutputsAndEndTurn(Game& game, shared_ptr<Player> me) {
    // Résoudre les conflits
    solveIssues();

   // Get all the directions
   //log::log("Real OUT");
   for (auto kv : shipDirectionQueue_) {
       Position shipPos = kv.first;
       shared_ptr<Ship> ship = gameMap_->at(shipPos)->ship;
       Direction dir = currentDirection(ship);
       command_queue_.push_back(ship->move(dir));
       //Position nextPos = destinationPos(ship);
       //log::log("ship " + to_string(ship->id) + " position " + ship->position.toString() +
       //         " -> " + nextPos.toString());
   }

   if (shouldMakeShip_) {
       if(isFreeSpace(me->shipyard->position)) {
           command_queue_.push_back(me->shipyard->spawn());
       }
   }

    return command_queue_;
}

bool StrategyMap::isFreeSpace(Position pos) {
    return shipsComingtoPos_[pos].size() == 0;
}


// Initialize allConflicts_ and call resolve all
void StrategyMap::solveIssues() {
    // Loop through the shipsComingtoPos_, obtain all conflicts
    // and keep it in a stack.

    for (auto kv : shipsComingtoPos_) {
        Position pos = kv.first;
        if (conflict(pos)) {
            allConflicts_.push(pos);
        }
    }
    iterateAndResolveConflicts();
}

// Does this block has conflict
bool StrategyMap::conflict(Position& pos) {
    if (collisionCenterOkay_)
    {
        // allow collision on dropoffs and shipyard
        vector<Position> homePositions;
        homePositions.push_back(me_->shipyard->position);

        for (auto dropoffPair : me_->dropoffs) {
        homePositions.push_back(dropoffPair.second->position);
        }

        for (Position homePos : homePositions) {
            if (homePos == pos) {
                return false;
            }
        }
    }

    if (shipsComingtoPos_[pos].size() >= 2) {
        return true;
    }

    if (nPlayers_ == 4) {
        if (pos != me_->shipyard->position) {
            if (shipsComingtoPos_[pos].size() >= 1) {

                if (hasEnemyShip(pos)) {
                    if (rng_() % 100 > 2) {
                        return true;
                    }
                }
                // if we already has a ship there, then
                // it is not a conflict!
                if (!gameMap_->at(pos)->is_occupied()) {
                     if(hasEnemyPresence(pos)) {
                    // in this case, we might hit an enemy. To be safe, we will say
                    // that we will have a 8% chance of not seeing the conflict.
                        if (rng_() % 100 > 8) {
                            return true;
                        }
                    }
                }
            }
        }
    }
    return false;
}

bool StrategyMap::hasEnemyShip(Position& pos) {
    MapCell* cell = gameMap_->at(pos);
    if (cell->is_occupied() && cell->ship->owner != me_->id) {
        return true;
    }
    return false;
}

bool StrategyMap::hasEnemyPresence(Position& centerPos) {
    for (Position surroundingPos : centerPos.get_surrounding_cardinals()) {
        surroundingPos = gameMap_->normalize(surroundingPos);
        MapCell* surroundingCell = gameMap_->at(surroundingPos);
        if (surroundingCell->is_occupied()) {
            if(surroundingCell->ship->owner != me_->id) {
                //log::log("enemy presence at " + centerPos.toString());
                return true;
            }
        }
    }
    return false;
}

void StrategyMap::iterateAndResolveConflicts() {

    while(!allConflicts_.empty()) {
        Position conflictMiddlePos = allConflicts_.front();
        allConflicts_.pop();
        //log::log("conflict: " + conflictMiddlePos.toString());
        solveIssue(conflictMiddlePos);
    }
}

void StrategyMap::solveIssue(Position middlePos) {
    //log::log("resolve: " + middlePos.toString());
    MapCell* middleCell = gameMap_->at(middlePos);
    // running into an enemy

    if (middleCell->is_occupied() && middleCell->ship->owner != me_->id) {
        //log::log("detected");
        vector<shared_ptr<Ship>> shipsToRedirect = shipsComingtoPos_[middlePos];
        redirectShips(shipsToRedirect);
    }

    // If the middle position is empty, we allow the
    // ship with the greatest halite to go to the destination,
    // but don't allow other ships.
    else if(!middleCell->is_occupied()) {
        if (hasEnemyPresence(middlePos)) {
            // avoid enemies case
            vector<shared_ptr<Ship>> shipsToRedirect = shipsComingtoPos_[middlePos];
            redirectShips(shipsToRedirect);
        }
        else {
            // in this case it is O -> X <- O
            // get all the ships pointing here
            vector<shared_ptr<Ship>> shipsToRedirect = shipsComingtoPos_[middlePos];
            auto maxShipIndex = max_element(shipsToRedirect.begin(), shipsToRedirect.end(),
                                            shipHasLessHalite);
            shared_ptr<Ship> maxShip = *maxShipIndex;
            shipsToRedirect.erase(maxShipIndex);

            redirectShips(shipsToRedirect);
        }
    }
    // O -> O <- O
        // If the middle position is not empty we allow the middle ship
        // to choose what to do, that is, we direct all other ships that the middle
        // ship is not pointing to.
        // If the middle ship chooses to stay still, then all other ships will have to
        // redirect.
    else {
        // if it is occupied, then it has a ship.
        // Case : it is not out ship
        shared_ptr<Ship> middleShip = gameMap_->at(middlePos)->ship;
        vector<shared_ptr<Ship>> shipsToRedirect = shipsComingtoPos_[middlePos];

        if (middleShip->owner != me_->id) {
            redirectShips(shipsToRedirect);
        }
        if(currentDirection(middleShip) == Direction::STILL) {
            redirectShips(shipsToRedirect);
        } else {
            // looking at where the current ship is pointing to
            Direction middleDir = currentDirection(middleShip);
            // the ship at the position can move to the middle, if it wants.
            // that is, it does not have to redirect if it is in the list.
            Position safeShipPos = gameMap_->destination_position(middlePos, middleDir);
            for (shared_ptr<Ship> shipToRedirect : shipsToRedirect) {
                if (shipToRedirect->position != safeShipPos) {
                    redirectShip(shipToRedirect);
                }
            }
        }
    }
}

void StrategyMap::redirectShip(shared_ptr<Ship> ship) {
    //log::log("ship " + to_string(ship->id) + " redirection:" + ship->position.toString());

    for(;;) {
        if( shipConflict(ship) ) {
            if(currentDirection(ship) != Direction::STILL) {
                changeToNextDirection(ship);
            } else {
                break;
            }
        } else {
            break;
        }
    }

    if(shipConflict(ship)) {
        allConflicts_.push(ship->position);
    }
}

void StrategyMap::redirectShips(vector<shared_ptr<Ship>> ships) {
    for (shared_ptr<Ship> ship : ships) {
        redirectShip(ship);
    }
}

// Change the direction of ship to be the next direction in queue
void StrategyMap::changeToNextDirection(shared_ptr<Ship> ship) {
    Position currentPos = ship->position;
    Position nextPos = destinationPos(ship);

    // remove
    auto eraseIndex = find(shipsComingtoPos_[nextPos].begin(), shipsComingtoPos_[nextPos].end(), ship);
    shipsComingtoPos_[nextPos].erase(eraseIndex);

    shipDirectionQueue_[currentPos].pop();

    // add : we have to get the destination again because
    // the current destination of the ship has changed by popping
    nextPos = destinationPos(ship);
    shipsComingtoPos_[nextPos].push_back(ship);
}

Position StrategyMap::destinationPos(shared_ptr<Ship> ship) {
    Direction currentDir = currentDirection(ship);
    Position targetDir = gameMap_->destination_position(ship->position, currentDir);
    return targetDir;
}

Direction StrategyMap::currentDirection(shared_ptr<Ship> ship) {
    Position shipPos = ship->position;
    if (shipDirectionQueue_[shipPos].empty()) {
        return Direction::STILL;
    }
    return shipDirectionQueue_[shipPos].front();
}

bool StrategyMap::shipHasLessHalite(const shared_ptr<Ship>& ship1, const shared_ptr<Ship>& ship2) {
    return ship1->halite < ship2->halite;
}

bool StrategyMap::shipConflict(shared_ptr<Ship> ship) {
    Position targetDir = destinationPos(ship);
    return conflict(targetDir);
}
