#include "World.h"
#include "Rabbit.h"
#include "Empty.h"
#include "Rock.h"
#include <iostream>
#include <algorithm>
#include <string>
#include <omp.h>
#include <map>
#include <vector>
#include <mutex>
using namespace std;

void World::printWorld() const {
    for (int i = 0; i < rows; ++i) {
        cout << "| ";
        for (int j = 0; j < cols; ++j) {
            Entity* e = grid[i][j];
            cout << e->getSymbol() << " | ";
        }
        cout << '\n';
    }
}

// Rabbits
void World::addRabbit(Rabbit* r) {
    rabbits.push_back(r);
}

void World::removeRabbit(Rabbit* r) {
    rabbits.erase(remove(rabbits.begin(), rabbits.end(), r), rabbits.end());
}

vector<pair<int, int>> World::getFreeAdjacentCells(int r, int c) {
    vector<pair<int, int>> cells;
    int dr[] = {-1, 0, 1, 0};  
    int dc[] = { 0, 1, 0, -1};

    for (int i = 0; i < 4; ++i) {
        int nr = r + dr[i];
        int nc = c + dc[i];
        if (nr >= 0 && nr < rows && nc >= 0 && nc < cols) {
            if (dynamic_cast<Empty*>(grid[nr][nc])) {
                cells.emplace_back(nr, nc);
            }
        }
    }
    return cells;
}

map<pair<int, int>, vector<Rabbit*>> movementPlans;
mutex mtx;

void World::planRabbitMovements() {
    movementPlans.clear();

    int total = rabbits.size();
    int num_threads = 0;

    // Vector de mapas locales por hilo
    std::vector<std::map<std::pair<int, int>, std::vector<Rabbit*>>> localPlans;

    #pragma omp parallel
    {
        int tid = omp_get_thread_num();

        #pragma omp single
        {
            num_threads = omp_get_num_threads();
            localPlans.resize(num_threads);
            std::cout << "Usando " << num_threads << " hilos.\n";
        }

        #pragma omp for schedule(static)
        for (int i = 0; i < total; ++i) {
            Rabbit* rabbit = rabbits[i];
            int r = rabbit->getRow();
            int c = rabbit->getCol();

            std::cout << "[Hilo " << tid << "] Rabbit en (" << r << "," << c << ")\n";

            auto options = getFreeAdjacentCells(r, c);
            int P = options.size();

            if (!options.empty()) {
                int destIndex = (getGeneration() + r + c) % P;
                auto dest = options[destIndex];
                localPlans[tid][dest].push_back(rabbit);
                std::cout << "[Hilo " << tid << "] Rabbit va a (" << dest.first << "," << dest.second << ")\n";
            } else {
                localPlans[tid][{r, c}].push_back(rabbit);
                std::cout << "[Hilo " << tid << "] Rabbit se queda en (" << r << "," << c << ")\n";
            }
        }
    }

    // Combinar todos los mapas locales de forma segura con un mutex
    for (int i = 0; i < num_threads; ++i) {
        for (const auto& entry : localPlans[i]) {
            lock_guard<mutex> lock(mtx);  
            movementPlans[entry.first].insert(
                movementPlans[entry.first].end(),
                entry.second.begin(),
                entry.second.end()
            );
        }
    }

    std::cout << "planRabbitMovements() terminado.\n";
}

void World::resolveRabbitMovements() {
    vector<Rabbit*> rabbitsAtStart = rabbits;
    for (auto& entry : movementPlans) {
        auto dest = entry.first;
        auto& contenders = entry.second;

        Rabbit* selected = contenders[0];
        for (Rabbit* r : contenders) {
            if (r->getAge() >= selected->getAge()) {
                selected = r;
            }
        }

        for (Rabbit* r : contenders) {
            if (r != selected) {
                int rR = r->getRow();
                int rC = r->getCol();
                handleRabbitReproduction(r,rR,rC);
                removeRabbit(r); 
            }
        }

        int oldR = selected->getRow();
        int oldC = selected->getCol();
        
        if (find(rabbitsAtStart.begin(), rabbitsAtStart.end(), selected) != rabbitsAtStart.end()) {
            handleRabbitReproduction(selected, oldR, oldC);
        }

        grid[dest.first][dest.second] = selected;
        selected->setPosition(dest.first, dest.second);

               
    }
}

void World::handleRabbitReproduction(Rabbit* rabbit, int oldR, int oldC) {
   
    if (rabbit->getAge() >= genProcRabbits) {
        Rabbit* baby = new Rabbit(oldR, oldC);
        addRabbit(baby);
        grid[oldR][oldC] = baby;
        rabbit->setAge(0);
        baby->setAge(0);
    } else {
        grid[oldR][oldC] = new Empty();
        rabbit->increaseAge();
    }
}

void World::printRabbitList(const vector<Rabbit*>& list, const string& label) const {
    cout << "[DEBUG] Lista de " << label << " (size=" << list.size() << "):\n";
    for (const Rabbit* r : list) {
        cout << " - Rabbit en (" << r->getRow() << "," << r->getCol()
                  << "), age=" << r->getAge()
                  << ", ptr=" << static_cast<const void*>(r) << "\n";
    }
}

//Foxes
void World::addFox(Fox* f) {
    foxes.push_back(f);
}

void World::removeFox(Fox* f) {
    foxes.erase(std::remove(foxes.begin(), foxes.end(), f), foxes.end());
}

vector<pair<int, int>> World::getAdjacentCellsWithRabbits(int r, int c) {
    vector<pair<int, int>> cells;
    int dr[] = {-1, 0, 1, 0};  
    int dc[] = { 0, 1, 0, -1};

    for (int i = 0; i < 4; ++i) {
        int nr = r + dr[i];
        int nc = c + dc[i];
        if (nr >= 0 && nr < rows && nc >= 0 && nc < cols) {
            if (dynamic_cast<Rabbit*>(grid[nr][nc])) {
                cells.emplace_back(nr, nc);
            }
        }
    }
    return cells;
}

map<pair<int, int>, vector<Fox*>> foxMovementPlans;
mutex foxMutex;

void World::planFoxMovements() {
    foxMovementPlans.clear();

    int total = foxes.size();
    int num_threads = 0;

    // Vector de mapas locales por hilo
    std::vector<std::map<std::pair<int, int>, std::vector<Fox*>>> localFoxPlans;

    #pragma omp parallel
    {
        int tid = omp_get_thread_num();

        #pragma omp single
        {
            num_threads = omp_get_num_threads();
            localFoxPlans.resize(num_threads);
            std::cout << "Usando " << num_threads << " hilos.\n";
        }

        #pragma omp for schedule(static)
        for (int i = 0; i < total; ++i) {
            Fox* fox = foxes[i];
            int r = fox->getRow();
            int c = fox->getCol();

            std::cout << "[Hilo " << tid << "] Fox en (" << r << "," << c << ")\n";

            auto rabbitCells = getAdjacentCellsWithRabbits(r, c);  
            if (!rabbitCells.empty()) {
                int P = rabbitCells.size();
                int destIndex = (getGeneration() + r + c) % P;
                auto dest = rabbitCells[destIndex];
                localFoxPlans[tid][dest].push_back(fox);
                std::cout << "[Hilo " << tid << "] Fox va a (" << dest.first << "," << dest.second << ")\n";
            } else {
                auto emptyCells = getFreeAdjacentCells(r, c);  
                if (!emptyCells.empty()) {
                    int P = emptyCells.size();
                    int destIndex = (getGeneration() + r + c) % P;
                    auto dest = emptyCells[destIndex];
                    localFoxPlans[tid][dest].push_back(fox);
                    std::cout << "[Hilo " << tid << "] Fox va a (" << dest.first << "," << dest.second << ")\n";
                } else {
                    localFoxPlans[tid][{r, c}].push_back(fox);
                    std::cout << "[Hilo " << tid << "] Fox se queda en (" << r << "," << c << ")\n";
                }
            }
        }
    }

    // Combinar todos los mapas locales de forma segura con un mutex
    for (int i = 0; i < num_threads; ++i) {
        for (const auto& entry : localFoxPlans[i]) {
            std::lock_guard<std::mutex> lock(foxMutex);  
            foxMovementPlans[entry.first].insert(
                foxMovementPlans[entry.first].end(),
                entry.second.begin(),
                entry.second.end()
            );
        }
    }

    std::cout << "planFoxMovements() terminado.\n";
}

void World::resolveFoxMovements() {
    vector<Fox*> foxesAtStart = foxes;

    for (auto& entry : foxMovementPlans) {
        auto dest = entry.first;
        auto& contenders = entry.second;

        Fox* selected = contenders[0];
        
        for (Fox* f : contenders) {
            if (f->getAge() > selected->getAge() || 
               (f->getAge() == selected->getAge() && f->getHunger() < selected->getHunger())) {
                selected = f;
            }
        }
        // Matar a los zorros que no fueron seleccionados
        for (Fox* f : contenders) {
            if (f != selected) {
                int rR = f->getRow();
                int rC = f->getCol();
                handleFoxReproduction(f,rR,rC);
                removeFox(f); 
            }
        }

        int oldR = selected->getRow();
        int oldC = selected->getCol();
        Entity* target = grid[dest.first][dest.second];
        if (Rabbit* prey = dynamic_cast<Rabbit*>(target)) {
            selected->setHunger(0);
            removeRabbit(prey);
            if (find(foxesAtStart.begin(), foxesAtStart.end(), selected) != foxesAtStart.end()) {
                handleFoxReproduction(selected, oldR, oldC);
            }
            grid[dest.first][dest.second] = selected;
            selected->setPosition(dest.first, dest.second);
        } else {
            selected->increaseHunger();
            if (selected->getHunger() == genFoodFoxes) {
                grid[oldR][oldC] = new Empty();
                removeFox(selected);
            }
            else{
                if (find(foxesAtStart.begin(), foxesAtStart.end(), selected) != foxesAtStart.end()) {
                    handleFoxReproduction(selected, oldR, oldC);
                }
        
                grid[dest.first][dest.second] = selected;
                selected->setPosition(dest.first, dest.second);
            }
        }
        
    }
}

void World::handleFoxReproduction(Fox* fox, int oldR, int oldC) {
    if (fox->getAge() >= genProcFoxes) {
        Fox* baby = new Fox(oldR, oldC);
        addFox(baby);
        grid[oldR][oldC] = baby;
        fox->setAge(0);
        baby->setAge(0);
    } else {
        grid[oldR][oldC] = new Empty();
        fox->increaseAge();
    }
}


void World::simulateGenerations() {
    for (int gen = 0; gen < nGen; ++gen) {
        planRabbitMovements();
        resolveRabbitMovements();
        planFoxMovements();          
        resolveFoxMovements();
        cout << "=== Generación " << gen+1 << " ===\n";
        printWorld();
        incrementGeneration();
    }
    printFinalState();
}

void World::printFinalState() const {
    int numObjetos = 0;

    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            Entity* e = grid[i][j];
            if (e != nullptr && !dynamic_cast<Empty*>(e)) {
                numObjetos++;
            }
        }
    }
  
    cout << genProcRabbits << " " << genProcFoxes << " " << genFoodFoxes << " "
         << 0 << " " << rows << " " << cols << " " << numObjetos << endl;

    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            Entity* e = grid[i][j];
            if (e != nullptr && !dynamic_cast<Empty*>(e)) {
                string tipo;
                if (dynamic_cast<Rock*>(e)) {
                    tipo = "ROCK";
                } else if (dynamic_cast<Rabbit*>(e)) {
                    tipo = "RABBIT";
                } else if (dynamic_cast<Fox*>(e)) {
                    tipo = "FOX";
                }
                cout << tipo << " " << i << " " << j << endl;
            }
        }
    }
}





