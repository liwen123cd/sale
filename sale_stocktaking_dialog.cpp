#include "sale_stocktaking_dialog.h"
#include "ui_sale_stocktaking_dialog.h"
#include "Create_Sale_DataBase.h"
#include <QSqlRecord>

Sale_Stocktaking_Dialog::Sale_Stocktaking_Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Sale_Stocktaking_Dialog)
{
    ui->setupUi(this);
    //时间初始化

    ui->Sale_dateEdit_start->setMaximumDateTime(QDateTime::currentDateTime());
    ui->Sale_dateEdit_end->setDateTime(QDateTime(QDate::currentDate(),QTime(23,59,59)));
    ui->Sale_dateEdit_end->setDateTimeRange(ui->Sale_dateEdit_start->dateTime(),
                                            QDateTime(QDate::currentDate(),QTime(23,59,59)));


    //商品编号初始化
    QStringList Sale_Item_ID_Lists;
    Sale_Item_ID_Lists<<QString("全部商品");
    ui->Sale_item_id_combobox->clear();
    ui->Sale_item_id_combobox->addItems(Sale_Item_ID_Lists);
    ui->Sale_item_id_combobox->setCurrentIndex(0);

    //根据时间，商品编号对账
    Sale_Stocktaking();

    connect(ui->Sale_dateEdit_start,SIGNAL(dateChanged(QDate)),this,SLOT(Sale_Set_Min_End_Date(QDate)));

}

Sale_Stocktaking_Dialog::~Sale_Stocktaking_Dialog()
{
    delete ui;
}

void Sale_Stocktaking_Dialog::Sale_Set_Min_End_Date(const QDate &dt)
{
    ui->Sale_dateEdit_end->setMinimumDate(dt);
}

void Sale_Stocktaking_Dialog::Sale_Stocktaking()
{
    QStringList sql;
    //获取库存管理中商品出库情况

    //统计订单中出库情况
    sql<<"select count(*) from Sale_State ";
    sql<<"join Sale_Order on ";
    sql<<"Sale_State.Sale_Order_ID=Sale_Order.Sale_Order_ID ";
    sql<<QString("where Sale_State.Sale_Order_State='出库' ");
    sql<<"and Sale_State.Sale_Date >= '";
    sql<<ui->Sale_dateEdit_start->dateTime().toString("yyyy-MM-dd hh:mm:ss");
    sql<<"' and Sale_State.Sale_Date <= '";
    sql<<ui->Sale_dateEdit_end->dateTime().toString("yyyy-MM-dd hh:mm:ss");
    sql<<"'";
    if(ui->Sale_item_id_combobox->currentIndex()!=0){
        sql<<QString(" and Sale_Order.Sale_Item_ID='%1'").arg(
                 ui->Sale_item_id_combobox->itemText(
                     ui->Sale_item_id_combobox->currentIndex()));
    }
    QSqlQuery query;
    query.exec(sql.join(""));
    qDebug()<<query.lastError();
    query.next();
    ui->Sale_lineEdit_order_out->setText(query.value(0).toString());
    //统计销售额（已完成）
    sql.clear();
    sql<<"select sum(Sale_Order.Sale_Item_Num * Sale_Order.Sale_Item_Price) from ";
    sql<<"Sale_Order join Sale_State on ";
    sql<<"Sale_State.Sale_Order_ID=Sale_Order.Sale_Order_ID ";
    sql<<"where Sale_Order.Sale_Order_Finished='1' ";
    sql<<QString("and Sale_State.Sale_Order_State='出库' ");
    sql<<"and Sale_State.Sale_Date >= '";
    sql<<ui->Sale_dateEdit_start->dateTime().toString("yyyy-MM-dd hh:mm:ss");
    sql<<"' and Sale_State.Sale_Date <= '";
    sql<<ui->Sale_dateEdit_end->dateTime().toString("yyyy-MM-dd hh:mm:ss");
    sql<<"'";
    if(ui->Sale_item_id_combobox->currentIndex()!=0){
        sql<<QString(" and Sale_Order.Sale_Item_ID='%1'").arg(
                 ui->Sale_item_id_combobox->itemText(
                     ui->Sale_item_id_combobox->currentIndex()));
    }
    query.exec(sql.join(""));
    qDebug()<<query.lastError();
    query.next();
    ui->Sale_lineEdit_turnover->setText(query.value(0).toString());
    //统计销量（已完成）
    sql.clear();
    sql<<"select sum(Sale_Order.Sale_Item_Num) from ";
    sql<<"Sale_Order join Sale_State on ";
    sql<<"Sale_State.Sale_Order_ID=Sale_Order.Sale_Order_ID ";
    sql<<"where Sale_Order.Sale_Order_Finished='1' ";
    sql<<QString("and Sale_State.Sale_Order_State='出库' ");
    sql<<"and Sale_State.Sale_Date >= '";
    sql<<ui->Sale_dateEdit_start->dateTime().toString("yyyy-MM-dd hh:mm:ss");
    sql<<"' and Sale_State.Sale_Date <= '";
    sql<<ui->Sale_dateEdit_end->dateTime().toString("yyyy-MM-dd hh:mm:ss");
    sql<<"'";
    if(ui->Sale_item_id_combobox->currentIndex()!=0){
        sql<<QString(" and Sale_Order.Sale_Item_ID='%1'").arg(
                 ui->Sale_item_id_combobox->itemText(
                     ui->Sale_item_id_combobox->currentIndex()));
    }
    query.exec(sql.join(""));
    qDebug()<<query.lastError();
    query.next();
    ui->Sale_lineEdit_volume->setText(query.value(0).toString());
}

void Sale_Stocktaking_Dialog::on_Sale_pushButton_check_clicked()
{
    Sale_Stocktaking();
}
