#ifndef EMPTY_H
#define EMPTY_H

#include "Entity.h"

class Empty : public Entity {
public:
    Empty() : Entity(' ') {} 
    
    char getSymbol() const override {
        return '.';
    }
};

#endif 