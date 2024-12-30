#ifndef WIDGETORDERS_H
#define WIDGETORDERS_H

#include <QtWidgets/qwidget.h>

namespace Ui {
class WidgetOrders;
}

class DialogCreateRefund;
class OrderManager;
class RefundManager;
class Order;
class WidgetOrders : public QWidget
{
    Q_OBJECT

public:
    explicit WidgetOrders(QWidget *parent = nullptr);
    ~WidgetOrders();
    void init(OrderManager *orderManager, RefundManager *refundManager);
    void clear();
    bool filterHideOrder(const Order *order) const;
    int getOrderCount() const;
    void hideVatColumns();

    OrderManager *orderManager() const;

public slots:
    void loadCustomerSettings();
    void filter();
    void resetFilter();
    void expandAllOrders();
    void addRefund();
    void updateShippingAddress();
    void cancelAddressChange();
    void exportShipments();

private:
    Ui::WidgetOrders *ui;
    void _connectSlots();
    OrderManager *m_orderManager;
    RefundManager *m_refundManager;
    DialogCreateRefund *m_dialogRefund;
};

#endif // WIDGETORDERS_H
