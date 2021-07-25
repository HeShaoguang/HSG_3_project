#ifndef BASE_H
#define BASE_H
#include <QPixmap>
#include <QRect>
#include <QVector>
#include <QObject>

class Base
{

public:
    Base();

    /* 更新坐标 */
    void updatePosition();

public:
    //资源对象
    QVector<QPixmap> m_pixArr;

    //坐标
    int m_X;
    int m_Y;
    //移动速度
    int m_Speed;
    //是否闲置
    bool m_Free;
    //矩形边框
    QRect m_Rect;
    //标记
    int direction;
};

#endif // BASE_H
