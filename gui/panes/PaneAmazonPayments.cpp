#include "PaneAmazonPayments.h"
#include "ui_PaneAmazonPayments.h"
#include "model/orderimporters/ImportedAmazonPaymentsReports.h"
#include "model/orderimporters/OrderManager.h"
#include "model/orderimporters/OrderImporterAmazonUE.h"
#include "model/orderimporters/ImportedFileReportManager.h"
#include "model/orderimporters/AmazonReportPaymentModel.h"

//==========================================================
PaneAmazonPayments::PaneAmazonPayments(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PaneAmazonPayments)
{
    ui->setupUi(this);
    ui->treeViewFiles->setModel(
                ImportedAmazonPaymentsReports::instance());
    m_orderManager = nullptr;
    ui->treeViewDetails->setModel(
                AmazonReportPaymentModel::instance());
    ui->treeViewDetails->header()->resizeSection(0, 200);
    _connectSlots();
}
//==========================================================
PaneAmazonPayments::~PaneAmazonPayments()
{
    delete ui;
}

//==========================================================
void PaneAmazonPayments::_connectSlots()
{
    //connect(ui->treeViewFiles->selectionModel(),
            //&QItemSelectionModel::selectionChanged,
            //this,
            //&PaneAmazonPayments::displayReport);
    connect(ui->buttonCompute,
            &QPushButton::clicked,
            this,
            &PaneAmazonPayments::compute);
}
//==========================================================
void PaneAmazonPayments::displayReport(
        const QItemSelection &newSelection,
        const QItemSelection &)
{
    // TODO display chart
}
//==========================================================
void PaneAmazonPayments::setVisible(bool visible)
{
    QWidget::setVisible(visible);
    if (visible == false) {
        m_yearsComputed.clear();
    }
}
//==========================================================
void PaneAmazonPayments::compute()
{
    setCursor(Qt::WaitCursor);
    if (m_orderManager != nullptr) {
        ui->treeViewOrders->setModel(nullptr);
        m_orderManager->deleteLater();
    }
    auto rows = ui->treeViewFiles->selectionModel()->selectedRows();
    if (rows.size() == 0) {
        // TODO clear table
    } else {
        auto index = rows.first();
        QString fileName = index.data().toString();
        QString importerName = OrderImporterAmazonUE().name();
        QString reportName = OrderImporterAmazonUE::REPORT_ORDERS_PAYMENTS;
        int year = index.parent().parent().data().toString().toInt();
        QString filePath = ImportedFileReportManager::instance()->
                filePath(importerName, reportName, fileName, year);
        CsvReader reader = OrderImporterAmazonUE().createAmazonReader(filePath);
        if (reader.readSomeLines(2)) {
            const DataFromCsv *dataRode = reader.dataRode();
            auto firstLineElements = reader.takeFirstLine();
            int indSettlementId = dataRode->header.pos("settlement-id");
            int indStartDate = dataRode->header.pos("settlement-start-date");
            int indEndDate = dataRode->header.pos("settlement-end-date");
            QDate minDate = OrderImporterAmazonUE::dateTimeFromString(
                        firstLineElements[indStartDate]).date();
            QDate maxDate = OrderImporterAmazonUE::dateTimeFromString(
                        firstLineElements[indEndDate]).date();
            QString paymentId = firstLineElements[indSettlementId];
            //*
            auto orders = ImportedFileReportManager::instance()->loadOrders(
                        importerName, minDate, maxDate);
            auto tempOrderManager = OrderManager::instance()->copyEmpty();
            tempOrderManager->recordOrders(importerName, *orders.data());
            //*/
            //m_orderManager = tempOrderManager->copyFilter(
                        //[paymentId](const Order * order) -> bool {
                //return order->paymentIds().contains(paymentId);
            //});
            if (!m_yearsComputed.contains(year)) {
                m_yearsComputed.clear(); /// We clear because VatOrdersModel can compute one year at a time
                VatOrdersModel::instance()->computeVat(year);
                m_yearsComputed << year;
            }
            m_orderManager = VatOrdersModel::instance()->orderManager()->copyFilter(
                        [paymentId](const Order * order) -> bool {
                return order->paymentIds().contains(paymentId);
            },
                        [paymentId](const Refund * refund) -> bool {
                return refund->getPaymentId() == paymentId;
            });
            if (minDate.year() != maxDate.year()) {
                m_yearsComputed.clear(); /// We clear because VatOrdersModel can compute one year at a time
                VatOrdersModel::instance()->computeVat(year-1);
                m_yearsComputed << year - 1;
                auto orderManagerBeforeOrder
                        = VatOrdersModel::instance()->orderManager()->copyFilter(
                            [paymentId, year](const Order * order) -> bool {
                    return order->paymentIds().contains(paymentId)
                            && !order->shipmentYears().contains(year);
                },
                [paymentId, year](const Refund * refund) -> bool {
                    return refund->getPaymentId() == paymentId
                            && refund->getDateTime().date().year() != year;
                });

                /*
                auto orders = ImportedFileReportManager::instance()->loadOrders(
                            importerName, minDate, maxDate);
                auto tempOrderManager = OrderManager::instance()->copyEmpty();
                tempOrderManager->recordOrders(importerName, *orders.data());
                //*/
                m_orderManager->recordOrders(orderManagerBeforeOrder);
            }
            // 1. Do back as before, but load order for all report types
            // 2. update compute funtion below
            // 3. Export accounting feature for both selected and whole month / whole year
            //tempOrderManager->deleteLater();
            ui->treeViewOrders->setModel(m_orderManager);
            ui->treeViewOrders->header()->resizeSection(0, 200);
            AmazonReportPaymentModel::instance()->compute(
                        m_orderManager, paymentId);
        }
    }
    setCursor(Qt::ArrowCursor);
}
//==========================================================
