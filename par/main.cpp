#include <iostream>
#include <fstream>
#include <sstream>
#include "World.h"
#include "Rock.h"
#include "Rabbit.h"
#include "Fox.h"
#include "Empty.h"  

using namespace std;
World* world;

void read(ifstream& input) {
    // Leer los parámetros en la primera línea
    int genProcRabbits, genProcFoxes, genFoodFoxes, nGen, rows, cols, nEntities;
    input >> genProcRabbits >> genProcFoxes >> genFoodFoxes >> nGen >> rows >> cols >> nEntities;

    world = new World(rows, cols);
    world->setGenProcRabbits(genProcRabbits);
    world->setGenProcFoxes(genProcFoxes);
    world->setGenFoodFoxes(genFoodFoxes);
    world->setNGen(nGen);
    world->setNEntities(nEntities);

    // Inicializar el mundo con Empty en todas las casillas
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            world->setEntityAt(i, j, new Empty());
        }
    }

    // Leer cada entidad
    string type;
    int r, c;
    for (int i = 0; i < nEntities; ++i) {
        input >> type >> r >> c;

        Entity* e = nullptr;
        if (type == "RABBIT") {
            Rabbit* rabbit = new Rabbit(r, c);
            e = rabbit;
            world->addRabbit(rabbit);  
        }
        else if (type == "FOX") {
            Fox* fox = new Fox(r, c);
            e = fox;
            world->addFox(fox);  
        }
        else if (type == "ROCK") e = new Rock(r,c);

        if (e != nullptr) {      
            world->setEntityAt(r, c, e);
        }
    }
    cout << "=== Generación " << 0 << " ===\n"; 
    world->printWorld();

}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Uso: %s <nombre_archivo.txt>\n", argv[0]);
        return 1;
    }

    ifstream archivo(argv[1]);
    if (!archivo.is_open()) {
        printf("No se pudo abrir el archivo: %s\n", argv[1]);
        return 1;
    }

    read(archivo);  
    archivo.close();
    world->simulateGenerations();
    delete world;
    return 0;
}
