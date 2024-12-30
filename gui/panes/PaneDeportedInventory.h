#ifndef PANEDEPORTEDINVENTORY_H
#define PANEDEPORTEDINVENTORY_H

#include <QWidget>

namespace Ui {
class PaneDeportedInventory;
}

class PaneDeportedInventory : public QWidget
{
    Q_OBJECT

public:
    explicit PaneDeportedInventory(QWidget *parent = nullptr);
    ~PaneDeportedInventory();

public slots:
    void resetSelection();
    void computeFromInventoryFiles();
    void onCustomerSelectedChanged(const QString &customerId);

private:
    Ui::PaneDeportedInventory *ui;
    void _connectSlots();
    void _disconnectSlots();
};

#endif // PANEDEPORTEDINVENTORY_H
