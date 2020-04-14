//BILLAUD Pierre / COUTURIEUX Victor STMN P5
#include "cellfriendliness.h"

CellFriendliness::CellFriendliness()
{
    finalNote = 0;
    totalHaliteArea = 0;
    enemiesInArea = 0;
    distanceWithShipyard = 0;
    position.x = 0;
    position.y = 0;
}

void CellFriendliness::evaluateCell(const int & bestHalite)
{
    // Plus la note est élevée, mieux c'est
    double noteEnemy = (20.0/(double)(enemiesInArea));
    double noteDistance = 5.0/(double)(distanceWithShipyard);

    if(enemiesInArea == 0) {
        noteEnemy = 21.0;
    }

    if(distanceWithShipyard == 0) {
        noteDistance = 6.0;
    }

    double noteHalite = 20.0 * ((double)totalHaliteArea/(double)bestHalite);

    finalNote = noteEnemy + noteDistance + noteHalite;
}

void CellFriendliness::setEnemiesInArea(const int & enemyCount)
{
    enemiesInArea = enemyCount;
}

void CellFriendliness::setTotalHaliteArea(const int & haliteAmount)
{
    totalHaliteArea = haliteAmount;
}

void CellFriendliness::setDistanceWithShipyard(const int & distance)
{
    distanceWithShipyard = distance;
}

void CellFriendliness::setPosition(const int &x, const int &y)
{
    position = Position(x, y);
}

int CellFriendliness::getEnemiesInArea() const
{
    return enemiesInArea;
}

int CellFriendliness::getTotalHaliteArea() const
{
    return totalHaliteArea;
}

int CellFriendliness::getDistanceWithShipyard() const
{
    return distanceWithShipyard;
}

double CellFriendliness::getFinaleNote() const
{
    return finalNote;
}

Position CellFriendliness::getPosition() const
{
    return position;
}
