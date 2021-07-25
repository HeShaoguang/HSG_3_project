#ifndef CONFIG_H
#define CONFIG_H

/********** 贪吃蛇游戏配置游戏配置数据 **********/
#define SNAKE_READ_TIME       400

/********** 游戏配置数据 **********/
#define GAME_WIDTH             472        //屏宽
#define GAME_HEIGHT            792       //屏高
#define GAME_TITLE           "协调游戏"   //标题
#define GAME_INTERVAL          15  //定时器间隔
#define GAME_LIFE              10  //初始生命值

/********** 地图配置数据 **********/
#define MAP_PATH ":/images/images/background2.jpg" //地图路径
#define MAP_SCROLL_SPEED        1 //地图滚动幅度

/********** 姿势配置数据 **********/
#define GESTURE_BETWEEN_SIZE    10
#define GESTURE_PATH     ":/images/red_%1.png" //爆炸路径
#define GESTURE_MAX              6   //爆炸图片最大索引
#define GESTURE_SIZE            100
#define GESTURE_INTERVAL        GESTURE_SIZE+GESTURE_BETWEEN_SIZE //第一列生成手势的出现间隔
#define GESTURE_PATH2     ":/images/green_%1.png" //爆炸路径
#define GESTURE_SIZE2            80
#define GESTURE_SPEED2            5
#define GESTURE_INTERVAL2        ((GESTURE_SIZE2+GESTURE_BETWEEN_SIZE)/GESTURE_SPEED2) //第一列输入手势的出现间隔
/********** 生成的手势配置数据 **********/
#define GESTURE_MAX              6   //爆炸图片最大索引
#define ITS_SIZE                15
#define My_SIZE                 15
/********** 每一列的位置 **********/
#define ROW1                     20
#define ROW2                     120
#define ROW3                     260
#define ROW4                     380

/********** 爆炸配置数据 **********/
#define BOMB_PATH     ":/images/images/bomb-%1.png" //爆炸路径
#define BOMB_MAX              7   //爆炸图片最大索引
#define BOMB_INTERVAL        10   //爆炸出现间隔
#define BOMB_NUM             20   //爆炸最大数量

#endif // CONFIG_H
