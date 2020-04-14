//BILLAUD Pierre / COUTURIEUX Victor STMN P5
#ifndef CELLFRIENDLINESS_H
#define CELLFRIENDLINESS_H
//Fichiers initiaux
#include "position.hpp"

#include <cassert>

using namespace hlt;

class CellFriendliness
{
public:
    CellFriendliness();

    void evaluateCell (const int & bestHalite);

    int getDistanceWithShipyard() const;
    int getTotalHaliteArea() const;
    int getEnemiesInArea() const;
    double getFinaleNote() const;
    Position getPosition() const;

    void setDistanceWithShipyard(const int & distance);
    void setTotalHaliteArea(const int & haliteAmount);
    void setEnemiesInArea(const int & enemyCount);
    void setPosition(const int &x, const int &y);

private:

    Position position;
    double finalNote;
    int distanceWithShipyard;
    int totalHaliteArea;
    int enemiesInArea;
};

#endif // CELLFRIENDLINESS_H
