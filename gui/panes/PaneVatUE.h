#ifndef PANEVATUE_H
#define PANEVATUE_H

#include <QtWidgets/qwidget.h>
#include <QtCore/qabstractitemmodel.h>
#include <QtGui/qbrush.h>

class OrderManager;
class RefundManager;
class Shipment;

namespace Ui {
class PaneVatUE;
}

class PaneVatUE : public QWidget
{
    Q_OBJECT

public:
    explicit PaneVatUE(QWidget *parent = nullptr);
    ~PaneVatUE();

public slots:
    void onCustomerSelectedChanged(const QString &customerId);
    void compute();
    void computeSelectedPeriod();
    void downloadCsvOrders();
    void downloadCsvVat();
    void downloadPdfReport();
    void erase();
    void displayOrderWithUncompleteReports(
            const QMap<QString, QList<QStringList>> &orders);
    void displayUncompleteShipments(
            const QMultiMap<QDateTime, Shipment *> &shipmentsAndRefunds);
    void displaySkusWithNoValuesFound(
            const QStringList &skus);
    void compareDifferencesWithAmazon();
    void browseInvoiceDir();
    void browseBookKeepingDir();

private:
    Ui::PaneVatUE *ui;
    void _connectSlots();
    void _loadSettings();
    OrderManager *m_orderManagerFilter;
    RefundManager *m_refundManager;
    bool _checkTableSelIndexes();
    void _generateTableData(const QModelIndexList &indexes);
    QList<QStringList> m_tableVatData;
    QList<QBrush> m_tableLineColors;
    QString m_tableRegime;
    QString m_tableCountryNameVat;
    QList<int> m_months;
    QString settingKeyCheckInvoice() const;
    QString settingKeyCheckBookKeeping() const;
    QString settingKeyDirInvoice() const;
    QString settingKeyDirBookKeeping() const;
    QString _createHtmlTable(const QList<QStringList> &tableWithHeader,
                             const QList<QBrush> &tableLineColors,
                             bool textAlignCenter = false);
    struct ColInfo {
        QString name;
        QString (*getValue)(const Shipment *shipmentOrRefund);
    };
    QList<ColInfo> colInfos() const;
};

#endif // PANEVATUE_H
