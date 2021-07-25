#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QWidget>
#include <QGraphicsView>
#include <QGraphicsProxyWidget>
#include "snakegame.h"
#include "gesturegame.h"
#include "databasewidget.h"
namespace Ui {
class MainWidget;
}

class MainWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MainWidget(QWidget *parent = nullptr);
    ~MainWidget();

private slots:
    void dealCloseSnake();
    void dealClosGesture();
    void dealCloseDatabse();
    void on_snakeButton_clicked();
    void on_gestureButton_clicked();

    void on_dataButton_clicked();

private:
    Ui::MainWidget *ui;
    SnakeGame *snakegame;
    GestureGame *gesturegame;
    QGraphicsView *view;
    QGraphicsScene *scene;
    QGraphicsProxyWidget *w;
    DatabaseWidget *databasewidget;
};

#endif // MAINWIDGET_H
