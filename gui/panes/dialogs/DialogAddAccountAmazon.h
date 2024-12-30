#ifndef DIALOGADDACCOUNTAMAZON_H
#define DIALOGADDACCOUNTAMAZON_H

#include <QDialog>

namespace Ui {
class DialogAddAccountAmazon;
}

class DialogAddAccountAmazon : public QDialog
{
    Q_OBJECT

public:
    explicit DialogAddAccountAmazon(QWidget *parent = nullptr);
    ~DialogAddAccountAmazon() override;
    void clear();
    QString getAmazon() const;
    QString getFAMAZON() const;
    QString getAccountCustomer() const;
    QString getAccountReserve() const;
    QString getAccountUnknownSales() const;
    QString getAccountCharge() const;

public slots:
    void accept() override;

private:
    Ui::DialogAddAccountAmazon *ui;
};

#endif // DIALOGADDACCOUNTAMAZON_H
