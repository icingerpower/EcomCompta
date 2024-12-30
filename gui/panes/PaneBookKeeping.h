#ifndef PANEBOOKKEEPING_H
#define PANEBOOKKEEPING_H

#include <QWidget>
#include <QTableView>
#include <QToolBox>
#include <QGroupBox>
#include <QSpacerItem>

class DialogAddInvoicePurchase;
class DialogAddInvoices;
class DialogAddBankAccount;
class DialogAddSelfEntry;

namespace Ui {
class PaneBookKeeping;
}

class PaneBookKeeping : public QWidget
{
    Q_OBJECT

public:
    explicit PaneBookKeeping(QWidget *parent = nullptr);
    ~PaneBookKeeping();

    static void initToolBoxBanks(
            QToolBox *toolBox,
            QGroupBox *groupBox,
            QSpacerItem *spacerToRemove);

public slots:
    void genBookKeeping();
    void checkBankBalances();
    void loadYearSelected();
    void unselectAll();

    void internBankToBank();

    void selfAssociate();
    void addSelfEntry();
    void removeSelfEntry();

    void associate();
    void dissociate();
    void hideAssociated(bool hide);

    void addInvoicePurchase();
    void addInvoicesPurchases();
    void removeInvoicePurchase();
    void viewPurchaseFile();

    void addInvoicePurchaseImport();
    void removeInvoicePurchaseImport();

    void addBankAccount();
    void removeBankAccount();
    void viewBankAccount();

private:
    Ui::PaneBookKeeping *ui;
    void _connectSlots();
    DialogAddInvoicePurchase *m_dialogAddInvoice;
    DialogAddInvoices *m_dialogAddInvoices;
    DialogAddBankAccount *m_dialogAddBankAccount;
    DialogAddSelfEntry *m_dialogAddSelfEntry;
    QList<QTableView *> _allEntryParserTableViews() const;
};

#endif // PANEBOOKKEEPING_H
