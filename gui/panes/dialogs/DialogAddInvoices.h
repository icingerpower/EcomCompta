#ifndef DIALOGADDINVOICES_H
#define DIALOGADDINVOICES_H

#include <QDialog>
#include "model/bookkeeping/entries/parsers/EntryParserPurchases.h"

namespace Ui {
class DialogAddInvoices;
}

class DialogAddInvoices : public QDialog
{
    Q_OBJECT

public:
    explicit DialogAddInvoices(QWidget *parent = nullptr);
    ~DialogAddInvoices();
    QStringList absFileNames() const;
    QList<PurchaseInvoiceInfo> getPurchaseInvoiceInfos() const;
    void clear();

public slots:
    void browseFilePaths();
    void accept() override;
    void reject() override;

private:
    Ui::DialogAddInvoices *ui;
};

#endif // DIALOGADDINVOICES_H
