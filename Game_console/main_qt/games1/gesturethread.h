#ifndef MYTHREAD_H
#define MYTHREAD_H

#include <QObject>

class GestureThread : public QObject
{
    Q_OBJECT
public:
    explicit GestureThread(QObject *parent = nullptr);

    void PAJ1Read();

    void setFlag(bool flag = true);

    int PAJ7620_Open1();
    int PAJ7620_Open2();

signals:
    void PAJ1Signal(unsigned char value1, unsigned char value2);

public slots:


private:
    bool isStop;
    int my_fd1;
    int my_fd2;
};

#endif // MYTHREAD_H
