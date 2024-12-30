#ifndef PaneServiceSales_H
#define PaneServiceSales_H

#include <QWidget>
#include <QSharedPointer>
#include "model/orderimporters/ServiceAccounts.h"

#include "PaneBookKeeping.h"

namespace Ui {
class PaneServiceSales;
}

class PaneServiceSales : public QWidget
{
    Q_OBJECT

public:
    explicit PaneServiceSales(QWidget *parent = nullptr);
    ~PaneServiceSales();

public slots:
    void loadYearSelected();
    void addBankAccount();
    void removeBankAccount();
    void viewBankAccount();

    void addAddress();
    void removeAddress();
    void saveAddress() const;
    void saveAddressInPosition(int index) const;
    void displayAddress(QItemSelection newSelection, QItemSelection previousSelection);

    void addSale();
    void addSaleFromBank();
    void removeSale();

private:
    QString _setSelServiceCustomer() const;
    QString _setSelServiceCustomerAddressId() const;
    Ui::PaneServiceSales *ui;
    void _connectSlots();
    QSharedPointer<ServiceAccounts> m_serviceAccounts;
};

#endif // PaneServiceSales_H
