#ifndef PARAMETERS_H
#define PARAMETERS_H
//BILLAUD Pierre / COUTURIEUX Victor STMN P5
#include <string>

using namespace std;

class Parameters
{
public:
    Parameters();

    void setFilepath(const string & val);
    void setMaxDistShipSee(const int & val);
    void setMaxHaliteShipMove(const int & val);
    void setMaxHaliteShipReturn(const int & val);
    void setNumberShipStopProduction(const int & val);
    void setTurnWhenBotStopSpawn(const int & val);

    string getFilepath();
    int getMaxDistShipSee();
    int getMaxHaliteShipMove();
    int getMaxHaliteShipReturn();
    int getNumberShipStopProduction();
    int getTurnWhenBotStopSpawn();

private:

    string filepath;
    int maximumDistanceShipSeesAround; // (1 à 32)
    int maximumHaliteShipMoveOnTile; // (0 à 1,000)
    int maximumHaliteShipReturn; // (0 à 1,000)
    int numberShipStopToProduce; // (1 à 100)
    int turnWhenBotDoesntSpawnShips; // (les tours vont de 1 à 500)
};

#endif // PARAMETERS_H
