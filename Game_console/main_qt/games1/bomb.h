#ifndef BOMB_H
#define BOMB_H
#include "base.h"

class Bomb : public Base
{
public:
    Bomb();
    void updatePosition();

    int m_Recoder;
};

#endif // BOMB_H
