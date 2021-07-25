#include "mygesture.h"
#include "config.h"


MyGesTure::MyGesTure()
{

    for(int i = 0;i<= GESTURE_MAX; i++)
    {
        QString str = QString(GESTURE_PATH2).arg(i);
        m_pixArr.push_back(QPixmap(str));
    }

    m_Speed = GESTURE_SPEED2;

    //子弹的矩形边框
    m_Rect.setWidth(GAME_WIDTH/2-ROW1);
    m_Rect.setHeight(GESTURE_SIZE2);
    m_X = ROW1;
    m_Y = GAME_HEIGHT;
    m_Rect.moveTo(m_X, m_Y+GAME_HEIGHT);
}

bool MyGesTure::updatePosition()
{
    //闲置不更新
    if(m_Free)
    {
        return false;
    }
    //Y坐标减
    m_Y -= m_Speed;
    m_Rect.moveTo(m_X, m_Y+GESTURE_SIZE2);

    //如果子弹射出屏幕更新为闲置
    if(m_Y < -GESTURE_SIZE2)
    {
        m_Free = true;
        return true;
    }
    return false;
}

bool MyGesTure::updatePosition2()
{
    //闲置不更新
    if(m_Free)
    {
        return false;
    }
    //Y坐标减
    m_Y -= m_Speed;
    m_Rect.moveTo(m_X-140, m_Y+GESTURE_SIZE2);

    //如果子弹射出屏幕更新为闲置
    if(m_Y < -GESTURE_SIZE2)
    {
        m_Free = true;
        return true;
    }
    return false;
}
