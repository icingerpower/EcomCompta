#ifndef PANECUSTOMERS_H
#define PANECUSTOMERS_H

#include <QWidget>
#include <QtCore/qitemselectionmodel.h>

#include "model/Customer.h"


class DialogNewCustomer;

namespace Ui {
class PaneCustomers;
}

class PaneCustomers : public QWidget
{
    Q_OBJECT

public:
    explicit PaneCustomers(QWidget *parent = nullptr);
    ~PaneCustomers();
    Customer getCustomerEdition() const;

public slots:
    void addCustomer();
    void addFirstCustomerForcing();
    void removeSelectedCustomer();
    void saveSelectedCustomer();
    void saveSelectedCustomer(QItemSelection selection);
    void displayCustomer(QItemSelection newSelection, QItemSelection previousSelection);

private:
    Ui::PaneCustomers *ui;
    void _connectSlots();
    void _showCustomerFields();
    void _hideCustomerFields();
    DialogNewCustomer *m_dialogNewCustomer;
};

#endif // PANECUSTOMERS_H
