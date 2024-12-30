#ifndef WIDGETADDRESS_H
#define WIDGETADDRESS_H

#include <QWidget>
#include "model/orderimporters/Address.h"

namespace Ui {
class WidgetAddress;
}

class WidgetAddress : public QWidget
{
    Q_OBJECT

public:
    explicit WidgetAddress(QWidget *parent = nullptr);
    ~WidgetAddress();
    QString getCountry() const;
    Address getAddress() const;
    void setAddress(const Address &address);
    bool isAddressComplete() const;

public slots:
    void clear();

private:
    Ui::WidgetAddress *ui;
};

#endif // WIDGETADDRESS_H
