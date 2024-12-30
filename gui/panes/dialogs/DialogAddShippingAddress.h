#ifndef DIALOGADDSHIPPINGADDRESS_H
#define DIALOGADDSHIPPINGADDRESS_H

#include <QtWidgets/qdialog.h>

#include "model/orderimporters/Address.h"

namespace Ui {
class DialogAddShippingAddress;
}

class DialogAddShippingAddress : public QDialog
{
    Q_OBJECT

public:
    explicit DialogAddShippingAddress(QWidget *parent = nullptr);
    ~DialogAddShippingAddress();
    Address getAddress();
    void clear();

    bool wasAccepted() const;

public slots:
    void accept() override;


private:
    Ui::DialogAddShippingAddress *ui;
    bool m_accepted;
};

#endif // DIALOGADDSHIPPINGADDRESS_H
