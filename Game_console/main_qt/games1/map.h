#ifndef MAP_H
#define MAP_H

#include <QPixmap>
#include <QPaintEvent>
#include <QTimer>

class Map
{
public:
    Map();
    //滚动坐标计算
    void mapPosition();
public:

    //图片对象
    QPixmap m_map1;
    QPixmap m_map2;
    //地图Y坐标
    int m_map1_posY;
    int m_map2_posY;
    //滚动幅度
    int m_scroll_speed;
};

#endif // MAP_H
