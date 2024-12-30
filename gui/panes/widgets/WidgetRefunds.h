#ifndef WIDGETREFUNDS_H
#define WIDGETREFUNDS_H

#include <QtWidgets/qwidget.h>

namespace Ui {
class WidgetRefunds;
}

class OrderManager;
class RefundManager;

class WidgetRefunds : public QWidget
{
    Q_OBJECT

public:
    explicit WidgetRefunds(QWidget *parent = nullptr);
    ~WidgetRefunds();
    void init(OrderManager *orderManager, RefundManager *refundManager);
    void clear();
    int getRefundCount() const;
    void hideVatColumns();

public slots:
    void removeSelectedRefunds();
    void expandAll();
    void exportCsv();
    void changeDate();

private:
    Ui::WidgetRefunds *ui;
    OrderManager *m_orderManager;
    RefundManager *m_refundManager;
};

#endif // WIDGETREFUNDS_H
