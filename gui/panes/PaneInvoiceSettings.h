#ifndef PANEINVOICESETTINGS_H
#define PANEINVOICESETTINGS_H

#include <QWidget>

#include "model/UpdateToCustomer.h"

namespace Ui {
class PaneInvoiceSettings;
}

class PaneInvoiceSettings : public QWidget, public UpdateToCustomer
{
    Q_OBJECT

public:
    explicit PaneInvoiceSettings(QWidget *parent = nullptr);
    void onCustomerSelectedChanged(
            const QString &customerId) override;
    QString uniqueId() const override;
    ~PaneInvoiceSettings();

public slots:
    void addDate();
    void removeDate();


private:
    Ui::PaneInvoiceSettings *ui;
    void _connectSlots();
    void _disconnectSlots();
    void _initPlainTextEdits();
};

#endif // PANEINVOICESETTINGS_H
