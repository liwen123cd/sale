#ifndef SALE_STOCKTAKING_DIALOG_H
#define SALE_STOCKTAKING_DIALOG_H

#include <QDateTime>
#include <QDialog>

namespace Ui {
class Sale_Stocktaking_Dialog;
}

class Sale_Stocktaking_Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Sale_Stocktaking_Dialog(QWidget *parent = 0);
    ~Sale_Stocktaking_Dialog();

private slots:
    void Sale_Set_Min_End_Date(const QDate &);
    void on_Sale_pushButton_check_clicked();

private:
    Ui::Sale_Stocktaking_Dialog *ui;
    void Sale_Stocktaking();
};

#endif // SALE_STOCKTAKING_DIALOG_H
