#include "mysqlquery.h"
#include <QSqlQuery>

MySqlQuery::MySqlQuery(QObject *parent) : QSqlQueryModel(parent)
{

}

QVariant MySqlQuery::data(const QModelIndex &index, int role) const
{
    QVariant value=QSqlQueryModel::data(index,role);

    if(role==Qt::DisplayRole && index.column()== 1 )
    {
        value=value.toString();
        if(value == "snake")
        {
            return "贪吃蛇游戏";
        }else if (value == "gesture") {
            return "协调游戏";
        }

        //qDebug() << value.toString();
        return value;
    }

    return value;
}

Qt::ItemFlags MySqlQuery::flags(const QModelIndex &index) const
{
    Qt::ItemFlags flags = QSqlQueryModel::flags(index);
//    if (index.column()!=6&&index.column()!=7) //可编辑状态
//       flags |= Qt::ItemIsEditable;
    return flags;
}
