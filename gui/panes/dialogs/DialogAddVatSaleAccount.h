#ifndef DIALOGADDVATSALEACCOUNT_H
#define DIALOGADDVATSALEACCOUNT_H

#include <QDialog>

namespace Ui {
class DialogAddVatSaleAccount;
}

class DialogAddVatSaleAccount : public QDialog
{
    Q_OBJECT

public:
    explicit DialogAddVatSaleAccount(QWidget *parent = nullptr);
    ~DialogAddVatSaleAccount();
    QString getRegime() const;
    QString getCountryName() const;
    QString getSaleType() const;
    double getVatRate() const;
    QString getVatRateString() const;
    QString getAccountSale() const;
    QString getAccountVat() const;

    bool wasAccepted() const;

public slots:
    void accept() override;
    void reject() override;
private:
    Ui::DialogAddVatSaleAccount *ui;
    bool m_wasAccepted;
};

#endif // DIALOGADDVATSALEACCOUNT_H
