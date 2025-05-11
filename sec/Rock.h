#ifndef ROCK_H
#define ROCK_H
#include "Entity.h"

class Rock : public Entity {
public:
    Rock(int r = 0, int c = 0) : Entity(r, c) {}
    char getSymbol() const override { return '*'; }
};

#endif
