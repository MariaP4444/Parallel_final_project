#ifndef ENTITY_H
#define ENTITY_H

class Entity {
private:
    int row;
    int col;

public:
    Entity(int r = 0, int c = 0) : row(r), col(c) {}

    void setPosition(int r, int c) {
        row = r;
        col = c;
    }

    int getRow() const { return row; }
    int getCol() const { return col; }

    virtual char getSymbol() const = 0; 
};

#endif
