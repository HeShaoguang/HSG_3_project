#ifndef MYSQLQUERY_H
#define MYSQLQUERY_H

#include <QObject>
#include <QSqlQueryModel>

class MySqlQuery : public QSqlQueryModel
{
public:

    MySqlQuery(QObject *parent);

    QVariant data(const QModelIndex &index, int role=Qt::DisplayRole) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
};

#endif // MYSQLQUERY_H
