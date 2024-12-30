#ifndef PANESHIPPINGADDRESSES_H
#define PANESHIPPINGADDRESSES_H

#include <QWidget>
#include <QtCore/qitemselectionmodel.h>

class DialogAddShippingAddress;

namespace Ui {
class PaneShippingAddresses;
}

class PaneShippingAddresses : public QWidget
{
    Q_OBJECT

public:
    explicit PaneShippingAddresses(QWidget *parent = nullptr);
    ~PaneShippingAddresses();

public slots:
    void addAddress();
    void removeAddress();
    void saveAddress() const;
    void saveAddressInPosition(int index) const;
    void displayAddress(QItemSelection newSelection, QItemSelection previousSelection);
    //void onCustomerSelectedChanged(const QString &customerName);

private:
    Ui::PaneShippingAddresses *ui;
    void _connectSlots();
    void _showAddressFields();
    void _hideAddressFields();
    DialogAddShippingAddress *m_dialogAddShippingAddress;
    QString m_settingKey;
};

#endif // PANESHIPPINGADDRESSES_H
