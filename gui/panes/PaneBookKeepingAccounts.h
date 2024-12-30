#ifndef PANEBOOKKEEPINGACCOUNTS_H
#define PANEBOOKKEEPINGACCOUNTS_H

#include <qstyleditemdelegate.h>
#include <QWidget>

#include "model/UpdateToCustomer.h"

class DialogAddAccountAmazon;


namespace Ui {
class PaneBookKeepingAccounts;
}

class PaneBookKeepingAccounts : public QWidget, public UpdateToCustomer
{
    Q_OBJECT

public:
    explicit PaneBookKeepingAccounts(
            QWidget *parent = nullptr);
    ~PaneBookKeepingAccounts() override;
    QString uniqueId() const override;
    void onCustomerSelectedChanged(
            const QString &customerId) override;

public slots:
    void browseBookKeepingDir();

    void addAccountAmazon();
    void removeAccountAmazon();
    void addAccountVatConf();
    void removeAccountVatConf();


private:
    Ui::PaneBookKeepingAccounts *ui;
    DialogAddAccountAmazon *m_dialogAddAccountAmazon;
    void _connectSlots();
};

class DelegateAccountsSalesUE : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit DelegateAccountsSalesUE(QObject *parent = nullptr);
    QWidget *createEditor(
            QWidget *parent,
            const QStyleOptionViewItem &option,
            const QModelIndex &index) const override;
    void setEditorData(QWidget *editor,
                       const QModelIndex &index) const override;
};



#endif // PANEBOOKKEEPINGACCOUNTS_H
