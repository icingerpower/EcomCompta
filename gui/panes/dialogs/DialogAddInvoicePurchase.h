#ifndef DIALOGADDINVOICEPURCHASE_H
#define DIALOGADDINVOICEPURCHASE_H

#include <QDialog>
#include "model/bookkeeping/entries/parsers/EntryParserPurchases.h"

namespace Ui {
class DialogAddInvoicePurchase;
}

class DialogAddInvoicePurchase : public QDialog
{
    Q_OBJECT

public:
    explicit DialogAddInvoicePurchase(QWidget *parent = nullptr);
    ~DialogAddInvoicePurchase();
    QString absFileName() const;
    PurchaseInvoiceInfo getPurchaseInvoiceInfo() const;
    void clear();

public slots:
    void accept() override;
    void browseFilePath();

private:
    Ui::DialogAddInvoicePurchase *ui;
};

#endif // DIALOGADDINVOICEPURCHASE_H
