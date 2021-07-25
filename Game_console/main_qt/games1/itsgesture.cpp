#include "itsgesture.h"
#include "config.h"

ItsGesture::ItsGesture()
{
    for(int i = 0;i<= GESTURE_MAX; i++)
    {
        QString str = QString(GESTURE_PATH).arg(i);
        m_pixArr.push_back(QPixmap(str));
    }

    m_Speed = MAP_SCROLL_SPEED;

    //子弹的矩形边框
    m_Rect.setWidth(GESTURE_SIZE);
    m_Rect.setHeight(GESTURE_SIZE);
    m_Rect.moveTo(m_X, m_Y);
}

/* 更新坐标 */
bool ItsGesture::updatePosition()
{
    //闲置不更新
    if(m_Free)
    {
        return false;
    }
    //Y坐标减
    m_Y += m_Speed;
    m_Rect.moveTo(m_X, m_Y);

    //如果出屏幕更新为闲置
    if(m_Y > GAME_HEIGHT)
    {
        m_Free = true;
        return true;
    }

    return false;
}
