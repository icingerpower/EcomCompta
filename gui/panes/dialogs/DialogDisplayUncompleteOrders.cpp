#include "DialogDisplayUncompleteOrders.h"
#include "ui_DialogDisplayUncompleteOrders.h"
#include "model/orderimporters/Shipment.h"
#include "model/orderimporters/Order.h"
#include "model/SettingManager.h"

//----------------------------------------------------------
DialogDisplayUncompleteOrders::DialogDisplayUncompleteOrders(
        QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogDisplayUncompleteOrders)
{
    ui->setupUi(this);
    m_nRows = 0;
}
//----------------------------------------------------------
DialogDisplayUncompleteOrders::~DialogDisplayUncompleteOrders()
{
    delete ui;
}
//----------------------------------------------------------
void DialogDisplayUncompleteOrders::setShipmentsRefunds(
        const QMultiMap<QDateTime, Shipment *> &shipmentsRefunds)
{
    auto colInfos = _colInfos();
    m_nRows = 0;
    int nCols = colInfos.size();
    auto dateFirstOfMonth = QDate::currentDate();
    dateFirstOfMonth.setDate(
                dateFirstOfMonth.year(),
                dateFirstOfMonth.month(),
                1);
    for (auto it = shipmentsRefunds.begin();
         it != shipmentsRefunds.end(); ++it) {
        auto activityDate = it.value()->getDateTime().date();
        if (activityDate < dateFirstOfMonth) {
            ++m_nRows;
        }
    }
    ui->tableWidget->setColumnCount(nCols);
    ui->tableWidget->setRowCount(m_nRows);
    QStringList labels;
    for (auto colInfo : colInfos) {
        labels << colInfo.name;
    }
    ui->tableWidget->setHorizontalHeaderLabels(labels);
    int idRow = 0;
    for (auto it = shipmentsRefunds.begin();
         it != shipmentsRefunds.end(); ++it) {
        auto shipmentOrRefund = it.value();
        auto activityDate = shipmentOrRefund->getDateTime().date();
        if (activityDate < dateFirstOfMonth) {
            for (int i=0; i<nCols; ++i) {
                QString value = colInfos[i].getValue(shipmentOrRefund);
                ui->tableWidget->setItem(
                            idRow, i,
                            new QTableWidgetItem(value));
            }
        }
        ++idRow;
    }
}
//----------------------------------------------------------
QList<DialogDisplayUncompleteOrders::ColInfo> DialogDisplayUncompleteOrders::_colInfos() const
{
    static QList<ColInfo> colInfos
            = {
        {tr("Id commande"), [](const Shipment *shipment) -> QString {
             return shipment->orderId();
         }}
        , {tr("Id activité"), [](const Shipment *shipment) -> QString {
             return shipment->orderId();
         }}
        , {tr("Channel"), [](const Shipment *shipment) -> QString {
             return shipment->channel();
         }}
        , {tr("Sous-channel"), [](const Shipment *shipment) -> QString {
             return shipment->subchannel();
         }}
        , {tr("Expédié vendeur"), [](const Shipment *shipment) -> QString {
               if (shipment->getOrder() == nullptr) {
                   return "???";
               }
             return shipment->getOrder()->getShippedBySeller() ? tr("OUI") : tr("NON");
         }}
        , {tr("Date"), [](const Shipment *shipment) -> QString {
             return shipment->getDateTime().toString(
               SettingManager::DATE_TIME_FORMAT_DISPLAY);
         }}
        , {tr("Montant TTC"), [](const Shipment *shipment) -> QString {
             return QString::number(shipment->getTotalPriceTaxed(), 'f', 2);
         }}
        , {tr("Remboursement"), [](const Shipment *shipment) -> QString {
             return shipment->isRefund() ? tr("REMBOURSEMENT") : tr("NON");
         }}
        , {tr("Rapports trouvés"), [](const Shipment *shipment) -> QString {
               QStringList reports;
               if (shipment->isRefund() && shipment->getOrder() == nullptr) {
                   reports << "Pas de commande complète trouvé";
               }
               auto reportsSet = shipment->getReportsFrom();
               for (auto report : reportsSet.keys()) {
                   reports << report;
               }

             return reports.join(" - ");
         }}
    };
    return colInfos;
}
//----------------------------------------------------------
int DialogDisplayUncompleteOrders::nRows() const
{
    return m_nRows;
}
//----------------------------------------------------------
