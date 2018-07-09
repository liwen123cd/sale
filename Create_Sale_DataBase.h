#ifndef CREATE_DATABASE_H
#define CREATE_DATABASE_H
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QDebug>
#include <QSqlError>

//执行sql语句
static int Sale_Sql(const QString&sql){
    QSqlQuery query;
    query.exec(sql);
    //qDebug()<<query.lastError();
    return query.lastError().number();
}

//创建，连接数据库
static bool Create_Sale_DataBase()
{
    //创建销售管理数据库

    QSqlDatabase Sale_db=QSqlDatabase::addDatabase("QSQLITE");
    Sale_db.setDatabaseName("Sale.db");

    if(!Sale_db.open())
    {
        qDebug()<<QObject::tr("销售管理数据库打开失败");
        return false;
    }

    //初始化订单数据库
    QString sql;
    sql="create table Sale_Order("
        "Sale_Order_ID varchar(30) primary key,"
        "Sale_Buyer_Name varchar(20),"
        "Sale_Buyer_Tel varchar(20),"
        "Sale_Buyer_Address varchar(40),"
        "Sale_Seller_ID int," //外键
        "Sale_Item_ID int,"//外键
        "Sale_Item_Num int,"
        "Sale_Item_Price float,"
        "Sale_Order_Finished int)";
    qDebug()<<Sale_Sql(sql);
    //插入一条记录
    /*sql="insert into Sale_Order values("
        "'160523212345',"
        "'tom',"
        "13456789999,"
        "'earth',"
        "0001,"
        "0001,"
        "3,"
        "34.56,"
        "0)";
    qDebug()<<Sale_Sql(sql);*/
    //初始化订单状态数据库
    sql="create table Sale_State("
        "Sale_State_ID integer primary key autoincrement,"
        "Sale_Order_ID varchar(30),"//外键
        "Sale_Order_State varchar(20),"
        "Sale_Date datetime,"
        "foreign key (Sale_Order_ID) "
        "references Sale_Order(Sale_Order_ID) on delete cascade)";
    qDebug()<<Sale_Sql(sql);
    sql="PRAGMA foreign_keys = ON";
    qDebug()<<Sale_Sql(sql);
    /*Sale_Query.exec("insert into Sale_State(Sale_Order_ID,Sale_Order_State,Sale_Date) values("
                    "'160523212345',"
                    "'create',"
                    "'2002-08-20 12:20:53.123'"
                    ")");
    qDebug()<<Sale_Query.lastError();*/

    return true;
}





#endif // CREATE_DATABASE_H
