#ifndef FOX_H
#define FOX_H

#include "Entity.h"

class Fox : public Entity {
private:
    int age;
    int hunger;  

public:
    Fox(int r = 0, int c = 0) : Entity(r, c), age(0), hunger(0) {}

    char getSymbol() const override { return 'F'; }

    int getAge() const { return age; }
    void setAge(int a) { age = a; }
    void increaseAge() { ++age; }

    int getHunger() const { return hunger; }
    void setHunger(int h) { hunger = h; }
    void increaseHunger() { ++hunger; }
    
};

#endif
