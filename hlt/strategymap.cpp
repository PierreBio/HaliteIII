//BILLAUD Pierre / COUTURIEUX Victor STMN P5
#include "strategymap.h"

#include <queue>
#include <stack>
#include <algorithm>
#include <random>

using namespace std;
using namespace hlt;

StrategyMap::StrategyMap(const shared_ptr<GameMap>& gameMap,const shared_ptr<Player>& me, const int & nPlayers, mt19937& rng, const bool & backToDropoff)
    :rng_(rng),
      me_(me),
      nPlayers_(nPlayers),
      gameMap_(gameMap),
      shouldMakeShip_(false),
      backToDropoff_(backToDropoff)
{
    shipDirectionQueue_ = {};
    vector<Command> command_queue_ = {};
    shipsComingtoPos_ = {};
    shipDirectionQueue_ = {};
    allConflicts_ = {};

    determineHaliteAreas();
}

void StrategyMap::determineHaliteAreas()
{

    int bestTotalHalite = 0;
    Position bestPosForHalite;

    Position shipyardPos = Position(me_->shipyard->position.x, me_->shipyard->position.y);
    int bestPositionDistance = gameMap_->width;

    //On regarde toutes les zones du jeu et on déduit laquelle possède le plus de halite
    for (int i = 0; i < gameMap_->height; i++) {
        for (int j = 0; j < gameMap_->width; j++) {

            int totalHaliteForCurrentArea = 0;
            MapCell * cell = gameMap_->at(Position(j,i));

            for(int h = i-1; h < i+2; h++) {
                for(int w = j-1; w < j+2; w++) {

                   Position normalized = gameMap_->normalize(Position(w,h));
                   int haliteAmount = totalHaliteForCurrentArea;
                   totalHaliteForCurrentArea = gameMap_->at(normalized)->halite + haliteAmount;
                }
            }

            int distanceWithShipyard = gameMap_->calculate_distance(shipyardPos,Position(j,i));

            //Quantité totale de Halite
            if(totalHaliteForCurrentArea > bestTotalHalite) {
                bestTotalHalite = totalHaliteForCurrentArea;
                bestPosForHalite = Position(j, i);
                bestPositionDistance = gameMap_->calculate_distance(shipyardPos,Position(j,i));
            }

            if(totalHaliteForCurrentArea == bestTotalHalite && distanceWithShipyard < bestPositionDistance) {
                bestPosForHalite = Position(j, i);
            }
        }
    }

    CellFriendliness * bestCellFriend = new CellFriendliness();

    //Choix de la première zone (on prend en compte les ennemis + la distance cette fois)
    for (int i = 0; i < gameMap_->height; i++) {
        for (int j = 0; j < gameMap_->width; j++) {

            int totalHaliteForCurrentArea = 0;
            int distanceWithShipyard = gameMap_->calculate_distance(shipyardPos,Position(j,i));
            int enemyCount = 0;

            for(int h = i-1; h < i+2; h++) {
                for(int w = j-1; w < j+2; w++) {
                    Position normalized = gameMap_->normalize(Position(w,h));
                    if(gameMap_->at(normalized)->is_occupied() &&
                        gameMap_->at(normalized)->ship->owner != me_->id) {
                        int currentEnemyCount = enemyCount;
                        enemyCount = currentEnemyCount + 1;
                    }

                    int haliteAmount = totalHaliteForCurrentArea;
                    totalHaliteForCurrentArea = gameMap_->at(normalized)->halite + haliteAmount;
                }
            }

            CellFriendliness * cellFriend = new CellFriendliness();
            cellFriend->setPosition(j, i);
            cellFriend->setDistanceWithShipyard(distanceWithShipyard);
            cellFriend->setTotalHaliteArea(totalHaliteForCurrentArea);
            cellFriend->setEnemiesInArea(enemyCount);
            cellFriend->evaluateCell(bestTotalHalite);

            if(cellFriend->getFinaleNote() > bestCellFriend->getFinaleNote()) {
                bestCellFriend = cellFriend;
            }
        }
    }

    targetArea = bestCellFriend;
}

CellFriendliness * StrategyMap::getTarget()
{
    return targetArea;
}

void StrategyMap::createDropoff(const shared_ptr<Ship> & ship)
{
    command_queue_.push_back(ship->make_dropoff());
}

bool StrategyMap::closeToTarget(const shared_ptr<Ship> & ship)
{
    if(gameMap_->calculate_distance(ship->position,targetArea->getPosition()) < 3) {
        return true;
    }

    return false;
}

void StrategyMap::createShip() {
    shouldMakeShip_ = true;
}

// Stocke la décision du bateau
void StrategyMap::stockDecision(const shared_ptr<Ship> & ship, vector<Direction> preferredDirs)
{
    shipIgnoresOpponent_[ship->position] = false;

    if (!gameMap_->can_move(ship)) {
        shipDirectionQueue_[ship->position].push(Direction::STILL);
        shipsComingtoPos_[ship->position].push_back(ship);
        return;
    }

    Position currentPos = Position(ship->position.x, ship->position.y);
    for (Direction dir : preferredDirs) {
        shipDirectionQueue_[ship->position].push(dir);
    }

    if (preferredDirs.empty()) {
        preferredDirs.push_back(Direction::STILL);
    }

    Direction mostPreferredDir = preferredDirs[0];
    Position newPos = gameMap_->destination_position(currentPos, mostPreferredDir);
    shipsComingtoPos_[newPos].push_back(ship);
}

vector<Command> StrategyMap::verifyAndRetrieveAllCommands(const Game& game,const shared_ptr<Player> & me)
{
    // Résoudre les conflits
    solveIssues();

    // Récapitulatif des directions
    for (auto kv : shipDirectionQueue_) {
       Position shipPos = kv.first;
       shared_ptr<Ship> ship = gameMap_->at(shipPos)->ship;
       Direction dir = destination(ship);
       command_queue_.push_back(ship->move(dir));
    }

    // Création d'un nouveau bateau ou non
    if (shouldMakeShip_) {
       if(noIntentToCreateShip(me->shipyard->position))
       {
           command_queue_.push_back(me->shipyard->spawn());
       }
    }

    return command_queue_;
}

bool StrategyMap::noIntentToCreateShip(Position pos)
{
    return shipsComingtoPos_[pos].size() == 0;
}

//Indique dans quelle direction est la destination du bateau
Direction StrategyMap::destination(const shared_ptr<Ship> & ship)
{
    Position shipPos = Position(ship->position.x, ship->position.y);
    if (shipDirectionQueue_[shipPos].empty()) {
        return Direction::STILL;
    }
    return shipDirectionQueue_[shipPos].front();
}

// Résoudre tous les conflits liés à la position en modifiant si besoin les destinations
void StrategyMap::solveIssues() {


    //Récapitule tous les conflits existants
    for (auto kv : shipsComingtoPos_) {
        Position pos = kv.first;
        if (conflict(pos)) {
            allConflicts_.push(pos);
        }
    }
    //Résoud ces derniers un par un
    iterateAndResolveConflicts();
}

// Vérifie si il y a un conflit
bool StrategyMap::conflict(const Position& pos)
{
    if (backToDropoff_)
    {
        // N'empêche pas le chevauchement entre bateau et base de collecte
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
                // Si le bateau est allié, alors ce n'est pas un conflit
                if (!gameMap_->at(pos)->is_occupied()) {

                     if(hasEnemyPresence(pos)) {
                    // 8% de chance de ne pas le voir, sinon on le frappe
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

//Vérifie si un ennemi se situe sur la position données
bool StrategyMap::hasEnemyShip(const Position& pos)
{
    MapCell* cell = gameMap_->at(pos);
    if (cell->is_occupied() && cell->ship->owner != me_->id) {
        return true;
    }
    return false;
}

//Vérifie si un ennmi se situe autour du bateau
bool StrategyMap::hasEnemyPresence(Position centerPos)
{
    for (Position surroundingPos : centerPos.get_surrounding_cardinals()) {
        surroundingPos = gameMap_->normalize(surroundingPos);
        MapCell* surroundingCell = gameMap_->at(surroundingPos);
        if (surroundingCell->is_occupied()) {
            if(surroundingCell->ship->owner != me_->id) {
                return true;
            }
        }
    }
    return false;
}

//Parcourt chaque signalement existant et les résoud un par un
void StrategyMap::iterateAndResolveConflicts()
{

    while(!allConflicts_.empty()) {
        Position conflictMiddlePos = allConflicts_.front();
        allConflicts_.pop();
        solveIssue(conflictMiddlePos);
    }
}

void StrategyMap::solveIssue(const Position & middlePos)
{

    MapCell* middleCell = gameMap_->at(middlePos);

    //Si la position est occupé par un ennemi on redirige les bateaux
    if (middleCell->is_occupied() && middleCell->ship->owner != me_->id) {
        vector<shared_ptr<Ship>> shipsToRedirect = shipsComingtoPos_[middlePos];
        redirectShips(shipsToRedirect);
    }

    //Si la position est vide, on permet au bateau le plus chargé en halite d'y aller
    else if(!middleCell->is_occupied()) {

        //Si il y a des ennemis pas loin de cette position, on redirige les bateaux
        if (hasEnemyPresence(middlePos)) {
            vector<shared_ptr<Ship>> shipsToRedirect = shipsComingtoPos_[middlePos];
            redirectShips(shipsToRedirect);
        } else {
            //Sinon, les bateaux y vont
            vector<shared_ptr<Ship>> shipsToRedirect = shipsComingtoPos_[middlePos];
            auto maxShipIndex = max_element(shipsToRedirect.begin(), shipsToRedirect.end(),
                                            shipHasLessHalite);
            shared_ptr<Ship> maxShip = *maxShipIndex;
            shipsToRedirect.erase(maxShipIndex);

            redirectShips(shipsToRedirect);
        }
    }

    // Si la position est occupé par un bateau allié, on regarde la décision qu'il a prise précèdemment
    // On ajuste la direction des autres bateaux en fonction de ce choix en évitant qu'ils passent au même endroit
    else {
        shared_ptr<Ship> middleShip = gameMap_->at(middlePos)->ship;
        vector<shared_ptr<Ship>> shipsToRedirect = shipsComingtoPos_[middlePos];

        if (middleShip->owner != me_->id) {
            redirectShips(shipsToRedirect);
        }

        if(destination(middleShip) == Direction::STILL) {
            redirectShips(shipsToRedirect);
        } else {
            // On regarde la destination du bateau
            Direction middleDir = destination(middleShip);
            Position safeShipPos = gameMap_->destination_position(middlePos, middleDir);

            for (shared_ptr<Ship> shipToRedirect : shipsToRedirect) {
                if (shipToRedirect->position != safeShipPos) {
                    redirectShip(shipToRedirect);
                }
            }
        }
    }
}

void StrategyMap::redirectShip(const shared_ptr<Ship> & ship)
{
    for(;;) {
        if( shipConflict(ship) ) {
            if(destination(ship) != Direction::STILL) {
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

void StrategyMap::redirectShips(vector<shared_ptr<Ship>> ships)
{
    for (shared_ptr<Ship> ship : ships) {
        redirectShip(ship);
    }
}

// Modifie la destination d'un bateau (réajuste le tableau des positions de destination)
void StrategyMap::changeToNextDirection(const shared_ptr<Ship> & ship)
{
    Position currentPos = ship->position;
    Position nextPos = destinationPos(ship);

    auto eraseIndex = find(shipsComingtoPos_[nextPos].begin(), shipsComingtoPos_[nextPos].end(), ship);
    shipsComingtoPos_[nextPos].erase(eraseIndex);

    shipDirectionQueue_[currentPos].pop();

    nextPos = destinationPos(ship);
    shipsComingtoPos_[nextPos].push_back(ship);
}

//Indique à quelle position est la destination du bateau
Position StrategyMap::destinationPos(const shared_ptr<Ship> & ship)
{
    Direction currentDir = destination(ship);
    Position targetDir = gameMap_->destination_position(ship->position, currentDir);
    return targetDir;
}

bool StrategyMap::shipHasLessHalite(const shared_ptr<Ship>& ship1, const shared_ptr<Ship>& ship2)
{
    return ship1->halite < ship2->halite;
}

bool StrategyMap::shipConflict(const shared_ptr<Ship> & ship)
{
    Position targetDir = destinationPos(ship);
    return conflict(targetDir);
}

