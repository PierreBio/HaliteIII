//BILLAUD Pierre / COUTURIEUX Victor STMN P5
#include "navigator.h"
#include "state.hpp"

#include <iostream>
#include <algorithm>
#include <cassert>

using namespace std;
using namespace hlt;

Navigator::Navigator(shared_ptr<GameMap>& gameMap, shared_ptr<Player>& me, unordered_map<EntityId, State>& stateShip, mt19937& rng, GeneticAlgo * geneticAlgo)
    : gameMap_(gameMap),
      me_(me),
      stateShip_(stateShip),
      rng_(rng),
      geneticAlgo_(geneticAlgo)
{

}

vector<Direction> Navigator::explore(const Position & position, shared_ptr<Ship> ship)
{
    vector<Direction> futureDir = findDirection(position, ship);

    return futureDir;
}

vector<Direction> Navigator::collect(shared_ptr<Ship> ship)
{
    return { Direction::STILL };
}

vector<Direction> Navigator::findDirection(const Position & position, shared_ptr<Ship> ship)
{

   if(position.y > ship->position.y) {
       if((position.y - ship->position.y) > (gameMap_->height/2) ) {
           return { Direction::NORTH };
       }

       if((position.y - ship->position.y) < (gameMap_->height/2) ) {
           return { Direction::SOUTH };
       }
   } else if(position.y < ship->position.y) {
       if(abs(position.y - ship->position.y) < (gameMap_->height/2) ) {
           return { Direction::NORTH };
       }

       if(abs(position.y - ship->position.y) > (gameMap_->height/2) ) {
           return { Direction::SOUTH };
       }
   }

    if(position.x > ship->position.x) {
        if((position.x - ship->position.x) > (gameMap_->width/2) ) {
            return { Direction::WEST };
        }

        if((position.x - ship->position.x) < (gameMap_->width/2) ) {
            return { Direction::EAST };
        }
    } else if (position.x < ship->position.x) {
        if(abs(position.x - ship->position.x) < (gameMap_->width/2) ) {
            return { Direction::WEST };
        }

        if(abs(position.x - ship->position.x) > (gameMap_->width/2) ) {
            return { Direction::EAST };
        }
    }

    return { Direction::STILL };
}

vector<Direction> Navigator::goToClosestDropoff(shared_ptr<Ship> ship)
{
    Position bestDropoffPos;
    int bestDistance = gameMap_->width;

    if(me_->dropoffs.size() > 0) {
        for (auto dropoffpair : me_->dropoffs) {
            Position dropoffPos = dropoffpair.second->position;
            int distance = gameMap_->calculate_distance(dropoffPos, ship->position);

            if(distance < bestDistance) {
                bestDistance = distance;
                bestDropoffPos = dropoffPos;
            }
        }

        return findDirection(bestDropoffPos, ship);
    }

    return findDirection(me_->shipyard->position, ship);
}
