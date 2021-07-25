#include "bomb.h"
#include "config.h"

Bomb::Bomb()
{
    for(int i = 1;i<= BOMB_MAX; i++)
    {
        QString str = QString(BOMB_PATH).arg(i);
        m_pixArr.push_back(QPixmap(str));
    }

    m_X = 0;
    m_Y = 0;
    m_Free = true;
    direction = 0;
    m_Recoder = 0;
}

void Bomb::updatePosition()
{
    if(m_Free)
    {
        return;
    }

    m_Recoder++;

    if(m_Recoder < BOMB_INTERVAL)
    {
        return;
    }

    m_Recoder = 0;

    direction++;

    if(direction > BOMB_MAX - 1)
    {
        direction = 0;
        m_Free = true;
    }
}
