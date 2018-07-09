#ifndef SALE_WIDGET_H
#define SALE_WIDGET_H

#include <QWidget>
#include <QDateTime>
#include <vector>

class QSqlTableModel;
class QSqlQueryModel;
class Sale_Detail_Dialog;
class QModelIndex;


struct Sale_Order_Detail;
/*{
    //订单信息
    int Sale_Row;
    QString Sale_Buyer_Name;
    QString Sale_Buyer_Tel;
    QString Sale_Buyer_Address;
    int Sale_Item_ID;
    int Sale_Item_Num;
    float Sale_Item_Price;//单价

};*/
struct Sale_State_Detail
{
    QString Sale_Order_ID;
    QString Sale_Order_State;
    QDateTime Sale_Date;
};
struct Product_Detail
{
    //商品信息
    QString Product_Name;
    float Product_Price;
    QString Product_Provider;

};


namespace Ui {
class Sale_Widget;
}

class Sale_Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Sale_Widget(QWidget *parent = 0);
    //订单状态修改函数（管理员用）(接口函数）
    static bool Sale_State_Change(const QString&,const QString&);
    //订单状态修改（仓库）
    static bool Sale_State_Out(const QString&);
    ~Sale_Widget();
protected:
    void closeEvent(QCloseEvent *);
signals:
    Sale_Send_Detail(const Sale_Order_Detail&);
    Sale_Save();
private slots:
    void on_Sale_pushButton_new_clicked();

    void on_Sale_pushButton_change_clicked();

    void on_Sale_pushButton_delete_clicked();

    void on_Sale_pushButton_select_order_clicked();

    void on_Sale_pushButton_select_number_clicked();

    void Sale_Recive_Detail(const Sale_Order_Detail&);

    void Sale_Cancel_New(const int);

    void Sale_Set_Min_End_Date(const QDate&);

    void on_Sale_pushButton_save_clicked();

    void on_Sale_puushButton_revoke_clicked();



    void on_tableView_doubleClicked(const QModelIndex &index);

    void on_Sale_pushButton_cancel_clicked();

    void on_Sale_pushButton_count_clicked();

    void on_Sale_pushButton_recive_clicked();

private:
    Ui::Sale_Widget *ui;
    QSqlTableModel *Sale_Table_Model;
    Sale_Detail_Dialog *Sale_Dialog;
    std::vector<Sale_State_Detail> Sale_State;
    //tableview初始化
    void Sale_New_Table();
    //临时保存订单状态修改（销售系统自用）
    void Sale_State_Order(const QString&,const QString&);
    //写到订单状态表
    void Sale_Save_Record();
    //创建订单号(未确定方法)
    void Sale_New_Order_ID(const QString&,QString&);
    //从表单读入结构体点击新建，修改按钮后执行
    bool Sale_Get_Order_Detail(Sale_Order_Detail&,int);
    //从结构体写入表单，dialog发送信号（带数据）后槽函数执行
    bool Sale_Write_Order_Detail(const Sale_Order_Detail&,int);
    //查询订单
    int Sale_Select_Order();
    //统计订单
    int Sale_Count_Order();
};

#endif // SALE_WIDGET_H
