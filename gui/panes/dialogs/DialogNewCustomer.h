#ifndef DIALOGNEWCUSTOMER_H
#define DIALOGNEWCUSTOMER_H

#include <QDialog>

#include "model/Customer.h"
#include "model/orderimporters/Address.h"

namespace Ui {
class DialogNewCustomer;
}

class DialogNewCustomer : public QDialog
{
    Q_OBJECT

public:
    explicit DialogNewCustomer(QWidget *parent = nullptr);
    ~DialogNewCustomer() override;
    Customer getCustomer() const;
    Address getAddress() const;
    void clear();
    void blockCancel();
    void releaseCancel();

public slots:
    void accept() override;
    void reject() override;

private:
    Ui::DialogNewCustomer *ui;
    bool m_blockCancel;
    bool m_closeMainWindowOnReject;
};

#endif // DIALOGNEWCUSTOMER_H
