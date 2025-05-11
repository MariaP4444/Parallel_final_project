#ifndef RABBIT_H
#define RABBIT_H

#include "Entity.h"

class Rabbit : public Entity {
private:
    int age;

public:
    Rabbit(int r = 0, int c = 0) : Entity(r, c), age(0) {}

    char getSymbol() const override { return 'R'; }

    int getAge() const { return age; }
    void setAge(int a) { age = a; }
    void increaseAge() { ++age; }
};

#endif
