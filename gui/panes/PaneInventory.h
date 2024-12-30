#ifndef PANEINVENTORY_H
#define PANEINVENTORY_H

#include <QWidget>
#include <QFileSystemModel>

namespace Ui {
class PaneInventory;
}

class DialogAddFileDate;

class PaneInventory : public QWidget
{
    Q_OBJECT

public:
    explicit PaneInventory(QWidget *parent = nullptr);
    ~PaneInventory();

public slots:
    void onCustomerSelectedChanged(const QString &customerId);

    void addPurchase();
    void addInventoryFile();
    void addReturns();
    void addCodesEquivalent();
    void addBundles();
    void removeSelectedFile();

    void exportUnkknownCodes();
    void exportErrorCodes();
    void exportFilteredInventory();

    void loadInventory();
    void saveInventory();
    void saveInventoryNotSoldForOneYear();
    void filter();
    void clearFilter();

private:
    Ui::PaneInventory *ui;
    void _connectSlots();
    QFileSystemModel *m_fileSystemModel;
    DialogAddFileDate *m_dialogAddInventory;
    DialogAddFileDate *m_dialogAddPurchase;
    DialogAddFileDate *m_dialogAddReturns;
    QString _settingKeySkusExclude() const;
    QSet<QString> _codesToExclude() const;
};

#endif // PANEINVENTORY_H
