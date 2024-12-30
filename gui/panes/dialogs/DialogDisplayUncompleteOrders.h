#ifndef DIALOGDISPLAYUNCOMPLETEORDERS_H
#define DIALOGDISPLAYUNCOMPLETEORDERS_H

#include <QDialog>
#include <QMultiMap>
#include <QDateTime>

namespace Ui {
class DialogDisplayUncompleteOrders;
}

class Shipment;

class DialogDisplayUncompleteOrders : public QDialog
{
    Q_OBJECT

public:
    explicit DialogDisplayUncompleteOrders(QWidget *parent = nullptr);
    ~DialogDisplayUncompleteOrders();
    void setShipmentsRefunds(
            const QMultiMap<QDateTime, Shipment *> &shipmentsRefunds);

    int nRows() const;

private:
    Ui::DialogDisplayUncompleteOrders *ui;
    struct ColInfo {
        QString name;
        QString (*getValue)(const Shipment *shipment);
    };
    QList<ColInfo> _colInfos() const;
    int m_nRows;

};

#endif // DIALOGDISPLAYUNCOMPLETEORDERS_H
