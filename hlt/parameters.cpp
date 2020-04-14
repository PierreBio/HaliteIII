//BILLAUD Pierre / COUTURIEUX Victor STMN P5
#include "parameters.h"

using namespace std;

Parameters::Parameters()
{

}

void Parameters::setFilepath(const string &val)
{
    filepath = val;
}

void Parameters::setMaxDistShipSee(const int &val)
{
    maximumDistanceShipSeesAround = val;
}

void Parameters::setMaxHaliteShipMove(const int &val)
{
    maximumHaliteShipMoveOnTile = val;
}

void Parameters::setMaxHaliteShipReturn(const int &val)
{
    maximumHaliteShipReturn = val;
}

void Parameters::setTurnWhenBotStopSpawn(const int &val)
{
    turnWhenBotDoesntSpawnShips = val;
}

void Parameters::setNumberShipStopProduction(const int &val)
{
    numberShipStopToProduce = val;
}

string Parameters::getFilepath()
{
    return filepath;
}

int Parameters::getMaxDistShipSee()
{
    return maximumDistanceShipSeesAround;
}

int Parameters::getMaxHaliteShipMove()
{
    return maximumHaliteShipMoveOnTile;
}

int Parameters::getMaxHaliteShipReturn()
{
    return maximumHaliteShipReturn;
}

int Parameters::getTurnWhenBotStopSpawn()
{
    return turnWhenBotDoesntSpawnShips;
}

int Parameters::getNumberShipStopProduction()
{
    return numberShipStopToProduce;
}
