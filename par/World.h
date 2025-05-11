#ifndef WORLD_H
#define WORLD_H

#include <vector>
#include "Entity.h"
#include <map>
#include <utility>
#include "Rabbit.h"
#include "Empty.h"
#include "Fox.h"

using namespace std;  

class World {
private:
    int genProcRabbits;
    int genProcFoxes;
    int genFoodFoxes;
    int nGen;
    int rows;
    int cols;
    int nEntities;

    int generation;

    vector<vector<Entity*>> grid;
    vector<Rabbit*> rabbits;
    vector<Fox*> foxes;



public:
    World(int r, int c)
        : rows(r), cols(c), genProcRabbits(0), genProcFoxes(0),
          genFoodFoxes(0), nGen(0), nEntities(0), generation(0),
          grid(r, vector<Entity*>(c, nullptr)), rabbits() {}

    // Setters
    void setGenProcRabbits(int v) { genProcRabbits = v; }
    void setGenProcFoxes(int v) { genProcFoxes = v; }
    void setGenFoodFoxes(int v) { genFoodFoxes = v; }
    void setGeneration(int gen) { generation = gen; }
    void setNGen(int v) { nGen = v; }
    void setRows(int r) { rows = r; grid.resize(r); }
    void setCols(int c) { cols = c; for (auto& row : grid) row.resize(c); }
    void setNEntities(int n) { nEntities = n; }

    // Getters
    int getGenProcRabbits() const { return genProcRabbits; }
    int getGenProcFoxes() const { return genProcFoxes; }
    int getGenFoodFoxes() const { return genFoodFoxes; }
    int getGeneration() const { return generation; }
    int getNGen() const { return nGen; }
    int getRows() const { return rows; }
    int getCols() const { return cols; }
    int getNEntities() const { return nEntities; }
    vector<Rabbit*> getRabbits() const { return rabbits; }

    // Grid
    Entity* getEntityAt(int r, int c) const { return grid[r][c]; }
    void setEntityAt(int r, int c, Entity* e) { grid[r][c] = e; }
    void printWorld() const;

    // Rabbits
    void addRabbit(Rabbit* r);
    void removeRabbit(Rabbit* r);
    void planRabbitMovements();
    void resolveRabbitMovements();
    vector<pair<int, int>> getFreeAdjacentCells(int r, int c);
    void handleRabbitReproduction(Rabbit* rabbit, int oldR, int oldC);
    void printRabbitList(const vector<Rabbit*>& list, const string& label) const;

    // Foxes
    void addFox(Fox* f);
    void removeFox(Fox* f);
    void planFoxMovements();
    void resolveFoxMovements();
    void handleFoxReproduction(Fox* fox, int oldR, int oldC);
    vector<pair<int, int>> getAdjacentCellsWithRabbits(int r, int c);

    // Generation
    void incrementGeneration() { generation++; }
    void simulateGenerations();
    void printFinalState() const;

    // Destructor para limpiar la memoria
    ~World() {
        for (auto& row : grid) {
            for (auto& cell : row) {
                delete cell;
            }
        }
    }
};

#endif
