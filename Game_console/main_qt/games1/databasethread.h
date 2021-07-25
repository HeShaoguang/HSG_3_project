#ifndef DATABASETHREAD_H
#define DATABASETHREAD_H

#include <QObject>

class DatabaseThread : public QObject
{
    Q_OBJECT
public:
    explicit DatabaseThread(QObject *parent = nullptr);
    void RFIDRead();

    void setFlag(bool flag = true);

    int RFID_Open();

signals:
    void RFIDSignal(QString value);

private:
    bool isStop;
    int my_fd;
};

#endif // DATABASETHREAD_H
