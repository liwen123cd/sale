#include "sale_widget.h"
#include "sale_detail_dialog.h"
#include "Create_Sale_DataBase.h"
#include "sale_stocktaking_dialog.h"
#include "ui_sale_widget.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlTableModel>
#include <QStringList>
#include <QDebug>
#include <QSqlRecord>
#include <QDateTime>
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlQueryModel>
#include <QHeaderView>
#include <QModelIndex>
#include <QPoint>

//构造函数
Sale_Widget::Sale_Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Sale_Widget)
{
    ui->setupUi(this);
    //初始化
    ui->Sale_pushButton_recive->hide();
    if(0){
        ui->Sale_pushButton_new->setEnabled(false);
        ui->Sale_pushButton_change->setEnabled(false);
        ui->Sale_pushButton_delete->setEnabled(false);
        ui->Sale_pushButton_cancel->setEnabled(false);
        ui->Sale_pushButton_recive->show();
    }
    Sale_New_Table();
    ui->tableView->setModel(Sale_Table_Model);
    //子窗口订单详细信息
    Sale_Dialog=new Sale_Detail_Dialog(this);
    Sale_Dialog->setModal(true);
    Sale_Dialog->hide();

    //子窗口销售记录盘点
    //信号与槽
    connect(this,SIGNAL(Sale_Send_Detail(Sale_Order_Detail)),Sale_Dialog,SLOT(Sale_Recive_Detail(Sale_Order_Detail)));
    connect(Sale_Dialog,SIGNAL(Sale_Send_Detail(Sale_Order_Detail)),this,SLOT(Sale_Recive_Detail(Sale_Order_Detail)));
    connect(Sale_Dialog,SIGNAL(Sale_Cancel(int)),this,SLOT(Sale_Cancel_New(int)));
    connect(ui->Sale_dateEdit_start,SIGNAL(dateChanged(QDate)),this,SLOT(Sale_Set_Min_End_Date(QDate)));
}
//添加订单物流记录
bool Sale_Widget::Sale_State_Change(const QString &Sale_Order_ID, const QString &Sale_Order_State)
{
    //根据订单号添加一条出库记录
    QStringList sql;
    sql<<"insert into Sale_State(Sale_Order_ID,Sale_Order_State,Sale_Date) values('";
    sql<<Sale_Order_ID;
    sql<<"','";
    sql<<Sale_Order_State;
    sql<<"','";
    sql<<QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    sql<<"')";
    //判断记录中是否已有该记录记录
    QSqlQuery query;
    query.exec(QString("select count(*) from Sale_State where "
                       "Sale_Order_ID='%1' and Sale_Order_State='%2'").arg(
                   Sale_Order_ID,Sale_Order_State));
    query.next();
    if(0!=query.record().value(0).toInt()){
        qDebug()<<tr("记录已存在");
        return false;
    }
    if(-1!=Sale_Sql(sql.join(""))){
        qDebug()<<tr("添加物流记录失败");
        return false;
    }
    if(Sale_Order_State==QString("签收")){
        Sale_Sql(QString("update Sale_Order set Sale_Order_Finished=1 where "
                         "Sale_Order_ID='%1'").arg(Sale_Order_ID));
    }
    return true;
}
//添加订单状态出库
bool Sale_Widget::Sale_State_Out(const QString &Sale_Order_ID)
{
    //根据订单号添加一条出库记录
    QStringList sql;
    sql<<"insert into Sale_State(Sale_Order_ID,Sale_Order_State,Sale_Date) values('";
    sql<<Sale_Order_ID;
    sql<<"','";
    sql<<QString("出库");
    sql<<"','";
    sql<<QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    sql<<"')";
    QSqlQuery query;
    query.exec(QString("select count(*) from Sale_State where "
                       "Sale_Order_ID='%1' and Sale_Order_State='出库'").arg(
                   Sale_Order_ID));
    query.next();
    if(0!=query.record().value(0).toInt()){
        qDebug()<<tr("记录已存在");
        return false;
    }
    if(-1!=Sale_Sql(sql.join(""))){
        qDebug()<<tr("添加出库记录失败");
        return false;
    }
    return true;
}

//析构函数
Sale_Widget::~Sale_Widget()
{
    delete ui;
}
//重写x事件，判断是否有更改
void Sale_Widget::closeEvent(QCloseEvent *event)
{
    if(!Sale_State.empty()){
        int ok=QMessageBox::information(this,tr("注意"),tr("是否保存修改"),QMessageBox::Yes,QMessageBox::No);
        if(ok==QMessageBox::Yes){
            on_Sale_pushButton_save_clicked();
        }else{
            on_Sale_puushButton_revoke_clicked();
        }
    }
    QWidget::closeEvent(event);
}


//新建订单
void Sale_Widget::on_Sale_pushButton_new_clicked()
{
    ui->tableView->setModel(Sale_Table_Model);
    //新建一条订单，弹出一个dialog，提示卖家输入+判断
    Sale_Order_Detail detail;
    //创建一条空记录
    int RowNum=Sale_Table_Model->rowCount();   
    Sale_Table_Model->insertRow(RowNum);
    //dialog读入空记录内容，暂定用结构体保存数据，用信号发数据
    Sale_Get_Order_Detail(detail,RowNum);
    detail.Sale_State=QString("创建订单");
    emit Sale_Send_Detail(detail);
    //显示订单详细页面
    Sale_Dialog->show();
}

//修改订单
void Sale_Widget::on_Sale_pushButton_change_clicked()
{
    ui->tableView->setModel(Sale_Table_Model);
    //和新建记录类似
    //修改一条订单，弹出一个dialog，提示卖家修改（和下面同一个dialog）
    Sale_Order_Detail detail;
    //获取当前选中行行数

    //判断，未选中不能修改，已删除不能修改，已完成不能修改
    int RowNum=ui->tableView->currentIndex().row();
    QSqlRecord record=Sale_Table_Model->record(RowNum);
    if(RowNum==-1){
        QMessageBox::warning(this,tr("警告"),tr("请选中一行"),QMessageBox::Ok);
    }else if(ui->tableView->currentIndex().data().isNull()){
        QMessageBox::warning(this,tr("警告"),tr("不能修改被删除项"),QMessageBox::Ok);
    }else if(record.value(8).toString()!="0"){
        QMessageBox::warning(this,tr("警告"),tr("不能修改已完成、取消项"),QMessageBox::Ok);
    }
    else{
        //dialog读入选中记录内容，暂定用结构体保存数据，用信号发数据
        Sale_Get_Order_Detail(detail,RowNum);
        detail.Sale_State=QString("修改订单");
        emit Sale_Send_Detail(detail);
        //显示订单详细页面
        Sale_Dialog->show();
    }

}

//删除订单
void Sale_Widget::on_Sale_pushButton_delete_clicked()
{
    ui->tableView->setModel(Sale_Table_Model);
    //删除一条订单，选中，删除,同时sale_state表中该订单所有信息
    int Row=ui->tableView->currentIndex().row();
    if(Row==-1){
        QMessageBox::warning(this,tr("警告"),tr("请选中一行"),QMessageBox::Ok);
    }else if(Sale_Table_Model->isDirty(ui->tableView->currentIndex())){
        QMessageBox::warning(this,tr("警告"),tr("请先保存修改项"),QMessageBox::Ok);
    }
    else{
        QSqlRecord record=Sale_Table_Model->record(Row);
        Sale_Table_Model->removeRow(Row);
    }
}
//取消订单
void Sale_Widget::on_Sale_pushButton_cancel_clicked()
{
    ui->tableView->setModel(Sale_Table_Model);
    //取消一条订单，选中，删除,同时sale_state表中该订单所有信息
    int Row=ui->tableView->currentIndex().row();
    QSqlRecord record=Sale_Table_Model->record(Row);
    QSqlQuery query;
    query.exec(QString("select count(*) from Sale_State where "
                       "Sale_Order_ID='%1' and Sale_Order_State="
                       "'%2'").arg(record.value(0).toString(),QString("出库")));
    qDebug()<<query.lastError();
    query.next();
    if(Row==-1){
        QMessageBox::warning(this,tr("警告"),tr("请选中一行"),QMessageBox::Ok);
    }else if(Sale_Table_Model->isDirty(ui->tableView->currentIndex())){
        QMessageBox::warning(this,tr("警告"),tr("请先保存修改项"),QMessageBox::Ok);
    }else if(record.value(8).toString()!="0"){
        QMessageBox::warning(this,tr("警告"),tr("不能取消已完成、取消项"),QMessageBox::Ok);
    }else if(query.record().value(0).toInt()!=0){
        QMessageBox::warning(this,tr("警告"),tr("订单已出库，无法取消"),QMessageBox::Ok);
    }else {
        record.setValue(8,2);
        Sale_Table_Model->setRecord(Row,record);
        Sale_State_Order(record.value(0).toString(),QString("取消订单"));
    }
}
//查询订单
void Sale_Widget::on_Sale_pushButton_select_order_clicked()
{
    //查订单，检查3个文本框里有没有内容，时间是否是有效时间，如果有，按内容查找，没有忽略
    //查找顺序时间，商品编号，买家手机号，订单号
    //目标，关键字搜索 
    Sale_Sql("create table temp("
             "Sale_Order_ID varchar(30) primary key,"
             "Sale_Item_ID int,"
             "Sale_Buyer_Tel varchar(20),"
             "Sale_Date datetime,"
             "Sale_Order_Finished int)");

    QStringList sql;    
    sql<<"insert into temp select ";
    sql<<"Sale_Order.Sale_Order_ID,Sale_Order.Sale_Item_ID,";
    sql<<"Sale_Order.Sale_Buyer_Tel,Sale_State.Sale_Date,Sale_Order.Sale_Order_Finished ";
    sql<<"from Sale_Order inner join Sale_State on ";
    sql<<"Sale_Order.Sale_Order_ID=Sale_State.Sale_Order_ID ";
    sql<<"where Sale_State.Sale_Date >= '";
    sql<<ui->Sale_dateEdit_start->dateTime().toString("yyyy-MM-dd hh:mm:ss");
    sql<<"' and Sale_State.Sale_Date <= '";
    sql<<ui->Sale_dateEdit_end->dateTime().toString("yyyy-MM-dd hh:mm:ss");
    sql<<QString("' and Sale_State.Sale_Order_State='创建订单'");
    if(1){
        //非管理员
        sql<<QString(" and Sale_Order.Sale_Seller_ID='%1'").arg("0001");

    }
    //关键字查询（以后），高亮（呵呵）
    if(ui->Sale_lineEdit_item_id->text()!=""){
        sql<<QString(" and Sale_Order.Sale_Item_ID='%1'").arg(
                 ui->Sale_lineEdit_item_id->text());
    }
    if(ui->Sale_lineEdit_buyer_tel->text()!=""){
        sql<<QString(" and Sale_Order.Sale_Buyer_Tel='%1'").arg(
                 ui->Sale_lineEdit_buyer_tel->text());
    }
    if(ui->Sale_lineEdit_order_id->text()!=""){
        sql<<QString(" and Sale_Order.Sale_Order_ID='%1'").arg(
                 ui->Sale_lineEdit_order_id->text());
    }
    sql<<" order by Sale_Order.Sale_Order_ID DESC";
    Sale_Sql(sql.join(""));

    QSqlQueryModel *Sale_Query_Model=new QSqlQueryModel(this);
    Sale_Query_Model->setQuery("select * from temp");
    Sale_Query_Model->setHeaderData(0,Qt::Horizontal,tr("订单号"));
    Sale_Query_Model->setHeaderData(1,Qt::Horizontal,tr("商品ID"));
    Sale_Query_Model->setHeaderData(2,Qt::Horizontal,tr("买家电话"));
    Sale_Query_Model->setHeaderData(3,Qt::Horizontal,tr("创建日期"));
    Sale_Query_Model->setHeaderData(4,Qt::Horizontal,tr("订单状态"));
    Sale_Query_Model->lastError();
    ui->tableView->setModel(Sale_Query_Model);

    Sale_Sql("drop table temp");
}
//查询销量
void Sale_Widget::on_Sale_pushButton_select_number_clicked()
{
    //查销量，同上，检查时间，商品编号，统计已完成订单中各种商品数量
    //查找顺序时间，商品编号
    Sale_Sql("create table temp("
             "Sale_Item_ID int,"
             "Sale_Item_Num int)");
    QStringList sql;
    sql<<"insert into temp select ";
    sql<<"Sale_Order.Sale_Item_ID,Sale_Order.Sale_Item_Num ";
    sql<<"from Sale_Order inner join Sale_State on ";
    sql<<"Sale_Order.Sale_Order_ID=Sale_State.Sale_Order_ID ";
    sql<<QString("and Sale_State.Sale_Order_State='%1' ").arg(QString("创建订单"));
    sql<<"where Sale_State.Sale_Date >= '";
    sql<<ui->Sale_dateEdit_start->dateTime().toString("yyyy-MM-dd hh:mm:ss");
    sql<<"' and Sale_State.Sale_Date <= '";
    sql<<ui->Sale_dateEdit_end->dateTime().toString("yyyy-MM-dd hh:mm:ss");
    sql<<"' and Sale_Order.Sale_Order_Finished=1";
    if(1){
        //非管理员
        sql<<QString(" and Sale_Order.Sale_Seller_ID='%1'").arg("0001");

    }
    if(ui->Sale_lineEdit_item_id->text()!=""){
        sql<<QString(" and Sale_Order.Sale_Item_ID='%1'").arg(
                 ui->Sale_lineEdit_item_id->text());
    }
    sql<<" order by Sale_Order.Sale_Item_ID";
    Sale_Sql(sql.join(""));

    QSqlQueryModel *Sale_Query_Model=new QSqlQueryModel(this);
    Sale_Query_Model->setQuery("select Sale_Item_ID,sum(Sale_Item_Num) as nums"
                               " from temp group by Sale_Item_ID");
    Sale_Query_Model->setHeaderData(0,Qt::Horizontal,tr("商品ID"));
    Sale_Query_Model->setHeaderData(1,Qt::Horizontal,tr("销量"));
    //Sale_Query_Model->lastError();
    ui->tableView_2->setModel(Sale_Query_Model);
    Sale_Sql("drop table temp");
}

//槽函数，接收从订单详细界面传回信息
void Sale_Widget::Sale_Recive_Detail(const Sale_Order_Detail &detail)
{
    //将结构体数据写入表单
    Sale_Write_Order_Detail(detail,detail.Sale_Row);

    //系统生成订单号，卖家id，订单状态
    QSqlRecord record=Sale_Table_Model->record(detail.Sale_Row);        
    //修改

    if(detail.Sale_Order_ID==""){

        //系统生成订单号(新建时)
        QString Order_ID;
        Sale_New_Order_ID(detail.Sale_Buyer_Tel,Order_ID);
        record.setValue(0,Order_ID);
        //输入卖家id
        record.setValue(4,0001);
        //状态修改为未完成（不可改）（不显示）
        record.setValue(8,0);
        Sale_Table_Model->setRecord(detail.Sale_Row,record);
        //添加修改记录
        Sale_State_Order(Order_ID,detail.Sale_State);
    }else{
        Sale_State_Order(detail.Sale_Order_ID,detail.Sale_State);
    }

}

//槽函数，新建订单时点击取消按钮删除新记录
void Sale_Widget::Sale_Cancel_New(const int Row)
{
    Sale_Table_Model->removeRow(Row);
}
//槽函数，设置结束时间最小值
void Sale_Widget::Sale_Set_Min_End_Date(const QDate &dt)
{
    ui->Sale_dateEdit_end->setMinimumDate(dt);
}

//创建订单号
void Sale_Widget::Sale_New_Order_ID(const QString &Buyer_Tel, QString &Order_ID)
{

    //根据卖家id，买家手机号，日期生成订单号
    //时间
    QDateTime current_time=QDateTime::currentDateTime();
    Order_ID+=current_time.toString("yyMMddhhmmss");
    //获取买家手机号
    Order_ID+=Buyer_Tel;
}

//将第Row行数据读入结构体
bool Sale_Widget::Sale_Get_Order_Detail(Sale_Order_Detail &Sale_Detail, int Row)
{
    //将第Row行数据读入结构体
    //qDebug()<<Sale_Table_Model->record(Row).value(0).toString();
    Sale_Detail.Sale_Order_ID=
            Sale_Table_Model->record(Row).value(0).toString();
    Sale_Detail.Sale_Buyer_Name=
            Sale_Table_Model->record(Row).value(1).toString();
    Sale_Detail.Sale_Buyer_Tel=
            Sale_Table_Model->record(Row).value(2).toString();
    Sale_Detail.Sale_Buyer_Address=
            Sale_Table_Model->record(Row).value(3).toString();
    Sale_Detail.Sale_Item_ID=
            Sale_Table_Model->record(Row).value(5).toInt();
    Sale_Detail.Sale_Item_Num=
            Sale_Table_Model->record(Row).value(6).toInt();
    Sale_Detail.Sale_Item_Price=
            Sale_Table_Model->record(Row).value(7).toFloat();
    Sale_Detail.Sale_Row=Row;
    return true;
}

//将结构体中数据写入第Row行
bool Sale_Widget::Sale_Write_Order_Detail(const Sale_Order_Detail &Detail, int Row)
{
    //将结构体中数据写入第Row行
    QSqlRecord record=Sale_Table_Model->record(Row);
    record.setValue(1,Detail.Sale_Buyer_Name);
    record.setValue(2,Detail.Sale_Buyer_Tel);
    record.setValue(3,Detail.Sale_Buyer_Address);
    record.setValue(5,Detail.Sale_Item_ID);
    record.setValue(6,Detail.Sale_Item_Num);
    record.setValue(7,Detail.Sale_Item_Price);
    Sale_Table_Model->setRecord(Row,record);
    return true;
}

//保存
void Sale_Widget::on_Sale_pushButton_save_clicked()
{
    //确认是否保存
    ui->tableView->setModel(Sale_Table_Model);
    int Yes=QMessageBox::information(this,tr("确认保存"),tr("确认保存"),QMessageBox::Yes,QMessageBox::No);
    if(Yes==QMessageBox::Yes){
        Sale_Table_Model->database().transaction();
        if(Sale_Table_Model->submitAll()){
            Sale_Table_Model->database().commit();

        }else{
            Sale_Table_Model->database().rollback();
            qDebug()<<"wrong";
        }
        Sale_Save_Record();
        on_Sale_puushButton_revoke_clicked();

    }
    //不保存恢复默认
}

//暂存一条修改记录
void Sale_Widget::Sale_State_Order(const QString &Order_ID, const QString &Order_State)
{
    Sale_State_Detail detail;
    detail.Sale_Date=QDateTime::currentDateTime();
    detail.Sale_Order_ID=Order_ID;
    detail.Sale_Order_State=Order_State;
    Sale_State.push_back(detail);
}

//保存修改记录
void Sale_Widget::Sale_Save_Record()
{
    //将Sale_State里的信息存入Sale_State表

    QStringList sql;
    //QSqlQuery query;
    for(std::vector<Sale_State_Detail>::iterator i=Sale_State.begin();
        i<Sale_State.end();++i){
        sql<<"insert into Sale_State(Sale_Order_ID,Sale_Order_State,Sale_Date) values('";
        sql<<i->Sale_Order_ID;
        sql<<"','";
        sql<<i->Sale_Order_State;
        sql<<"','";
        sql<<i->Sale_Date.toString("yyyy-MM-dd hh:mm:ss");
        sql<<"')";
        //如果状态是创建订单，调用仓库出库函数,根据订单号读商品id，数量
        if(i->Sale_Order_State=="创建订单"){
            int row=0;
            while(1){
                if(Sale_Table_Model->record(row).value(0).toString()==
                        i->Sale_Order_ID){
                    break;
                }
                ++row;
            }

            QSqlRecord record=Sale_Table_Model->record(row);
            //qDebug()<<record.value(0).toString();
            //调用出库函数
        }

        if(-1!=Sale_Sql(sql.join(""))){
            QMessageBox::warning(this,tr("警告"),sql.join(""),QMessageBox::Ok);
        }
        sql.clear();
    }
    Sale_State.clear();
    //记录重新排序(暂时不做)
}

//撤销所有修改
void Sale_Widget::on_Sale_puushButton_revoke_clicked()
{
    //撤销所有修改
    Sale_State.clear();
    Sale_New_Table();
    ui->tableView->setModel(Sale_Table_Model);

}
//初始化
void Sale_Widget::Sale_New_Table()
{
    //清空已存在信息
    ui->Sale_lineEdit_buyer_tel->clear();
    ui->Sale_lineEdit_item_id->clear();
    ui->Sale_lineEdit_order_id->clear();
    //时间初始化
    ui->Sale_dateEdit_start->setDateTimeRange(ui->Sale_dateEdit_start->dateTime(),
                                              QDateTime::currentDateTime());
    ui->Sale_dateEdit_end->setDateTime(QDateTime(QDate::currentDate(),QTime(23,59,59)));

    ui->Sale_dateEdit_end->setDateTimeRange(ui->Sale_dateEdit_start->dateTime(),
                                            QDateTime(QDate::currentDate(),QTime(23,59,59)));
    //table modelc初始化
    Sale_Table_Model=new QSqlTableModel(this);
    Sale_Table_Model->setTable("Sale_Order");
    if(1){
        //如果不是管理员，显示该卖家订单
        Sale_Table_Model->setFilter(QString("Sale_Seller_ID='%1'").arg("0001"));
    }
    Sale_Table_Model->setSort(0,Qt::DescendingOrder);
    Sale_Table_Model->select();

    //tablemodel样式设置
    Sale_Table_Model->setEditStrategy(QSqlTableModel::OnManualSubmit);
    Sale_Table_Model->setHeaderData(0,Qt::Horizontal,tr("订单号"));
    Sale_Table_Model->setHeaderData(1,Qt::Horizontal,tr("买家姓名"));
    Sale_Table_Model->setHeaderData(2,Qt::Horizontal,tr("买家手机号"));
    Sale_Table_Model->setHeaderData(3,Qt::Horizontal,tr("买家地址"));
    Sale_Table_Model->setHeaderData(4,Qt::Horizontal,tr("卖家ID"));
    Sale_Table_Model->setHeaderData(5,Qt::Horizontal,tr("商品ID"));
    Sale_Table_Model->setHeaderData(6,Qt::Horizontal,tr("商品数量"));
    Sale_Table_Model->setHeaderData(7,Qt::Horizontal,tr("商品售价"));
    Sale_Table_Model->setHeaderData(8,Qt::Horizontal,tr("订单状态"));

    on_Sale_pushButton_select_number_clicked();
    ui->tableView->horizontalHeader()->setStretchLastSection(true);
    ui->tableView->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
    ui->tableView_2->horizontalHeader()->setStretchLastSection(true);
    ui->tableView_2->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);

}
//槽函数，双击事件
void Sale_Widget::on_tableView_doubleClicked(const QModelIndex &index)
{
    int Row=0;
    while(1){
       if(Sale_Table_Model->record(Row).value(0).toString()==
               index.sibling(index.row(),0).data().toString()){
           break;
       }
       ++Row;
    }
    qDebug()<<Row;
    Sale_Order_Detail detail;
    QSqlRecord record=Sale_Table_Model->record(Row);
    Sale_Get_Order_Detail(detail,Row);
    //订单只能查看
    detail.Sale_State=QString("查看订单");
    emit Sale_Send_Detail(detail);
    //显示订单详细页面
    Sale_Dialog->show();

}

//销售记录盘点
void Sale_Widget::on_Sale_pushButton_count_clicked()
{
    Sale_Stocktaking_Dialog * dialog=new Sale_Stocktaking_Dialog(this);
    dialog->show();
}

void Sale_Widget::on_Sale_pushButton_recive_clicked()
{
    Sale_State_Change(ui->tableView->currentIndex().sibling(
                          ui->tableView->currentIndex().row(),0).data().toString(),
                      QString("签收"));
}
