#ifndef DIALOGCREATEREFUND_H
#define DIALOGCREATEREFUND_H

#include <QtWidgets/qdialog.h>


class Order;
class Refund;
class RefundManager;

namespace Ui {
class DialogCreateRefund;
}

class DialogCreateRefund : public QDialog
{
    Q_OBJECT

public:
    explicit DialogCreateRefund(
            Order *order,
            RefundManager *refundManager,
            QWidget *parent = nullptr);
    ~DialogCreateRefund() override;
    Refund *createRefund(); /// Gives ownership
    void clear();

public slots:
    void accept() override;

private:
    Ui::DialogCreateRefund *ui;
    Order *m_order;
    RefundManager *m_refundManager;
};

#endif // DIALOGCREATEREFUND_H
