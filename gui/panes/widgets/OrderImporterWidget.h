#ifndef ORDERIMPORTERWIDGET_H
#define ORDERIMPORTERWIDGET_H

#include <QWidget>
#include "model/orderimporters/AbstractOrderImporter.h"
#include "model/UpdateToCustomer.h"

class DialogCreateRefund;

namespace Ui {
class OrderImporterWidget;
}

class OrderImporterWidget : public QWidget, public UpdateToCustomer
{
    Q_OBJECT

public:
    explicit OrderImporterWidget(QWidget *parent = nullptr);
    ~OrderImporterWidget() override;
    void init(AbstractOrderImporter *importer);
    bool filterHideOrder(const Order *order) const;
    void onCustomerSelectedChanged(const QString &customerId) override;
    QString uniqueId() const override;

public slots:
    void loadImportersSettings();
    /*

    /// Action in order tab
    void filter();
    void resetFilter();
    void expandAllOrders();
    void addRefund();
    void updateShippingAddress();
    void cancelAddressChange();

    /// Action in order tab
    void removeSelectedRefunds();
    //*/

private:
    Ui::OrderImporterWidget *ui;
    void _connectSlots();
    AbstractOrderImporter *m_importer;
    DialogCreateRefund *m_dialogRefund;
    bool m_changingCustomer;
};

#endif // ORDERIMPORTERWIDGET_H
