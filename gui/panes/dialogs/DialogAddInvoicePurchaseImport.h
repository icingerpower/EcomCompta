#ifndef DIALOGADDINVOICEPURCHASEIMPORT_H
#define DIALOGADDINVOICEPURCHASEIMPORT_H

#include <QDialog>

#include "model/bookkeeping/entries/parsers/EntryParserImportations.h"

namespace Ui {
class DialogAddInvoicePurchaseImport;
}

class DialogAddInvoicePurchaseImport : public QDialog
{
    Q_OBJECT

public:
    explicit DialogAddInvoicePurchaseImport(
            QWidget *parent = nullptr);
    ~DialogAddInvoicePurchaseImport();
    QString absFileName() const;
    ImportInvoiceInfo getImportInvoiceInfo() const;
    void clear();
    bool wasAccepted() const;

public slots:
    void accept() override;
    void reject() override;
    void browseFilePath();

private:
    Ui::DialogAddInvoicePurchaseImport *ui;
    bool m_wasAccepted;
};

#endif // DIALOGADDINVOICEPURCHASEIMPORT_H
