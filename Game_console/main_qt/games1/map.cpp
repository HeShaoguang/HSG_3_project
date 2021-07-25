#include "map.h"
#include "config.h"

Map::Map()
{
    //地图图片对象
    m_map1.load(MAP_PATH);
    m_map2.load(MAP_PATH);

    //初始化地图坐标
    m_map1_posY = -GAME_HEIGHT;
    m_map2_posY = 0;

    //设置滚动幅度
    m_scroll_speed = MAP_SCROLL_SPEED;
}

void Map::mapPosition()
{
    //地图坐标增加
    m_map1_posY += MAP_SCROLL_SPEED;
    m_map2_posY += MAP_SCROLL_SPEED;

    //地图到头时调整
    if(m_map1_posY >= 0)
    {
        m_map1_posY = -GAME_HEIGHT;
    }

    if(m_map2_posY >= GAME_HEIGHT)
    {
        m_map2_posY = 0;
    }
}
