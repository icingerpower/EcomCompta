#include <QInputDialog>
#include <QtCharts/qlineseries.h>
#include <QtCharts/qdatetimeaxis.h>
#include <QtCharts/qvalueaxis.h>
#include <QtCharts/qchart.h>

#include "model/orderimporters/VatOrdersModel.h"
#include "model/orderimporters/SkuFoundTableModel.h"
#include "model/orderimporters/SkusGroupProfitModel.h"
#include "model/orderimporters/ImporterYearsManager.h"
#include "model/orderimporters/Shipment.h"
#include "model/orderimporters/Order.h"

#include "PaneProfit.h"
#include "ui_PaneProfit.h"

using namespace QtCharts;

//----------------------------------------------------------
PaneProfit::PaneProfit(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PaneProfit)
{
    ui->setupUi(this);
    ui->comboBoxYear->setModel(ImporterYearsManager::instance());
    ui->tableKnownSkus->setModel(
                SkuFoundTableModel::instance());
    ui->listViewGroups->setModel(
                SkusGroupProfitModel::instance());
    _connectSlots();
}
//----------------------------------------------------------
void PaneProfit::_connectSlots()
{
    connect(ui->buttonCompute,
            &QPushButton::clicked,
            this,
            &PaneProfit::compute);
    connect(ui->buttonLoadYearSkus,
            &QPushButton::clicked,
            this,
            &PaneProfit::loadYearSkus);
    connect(ui->buttonAddSku,
            &QPushButton::clicked,
            this,
            &PaneProfit::addSku);
    connect(ui->buttonFilter,
            &QPushButton::clicked,
            this,
            &PaneProfit::filterSkus);
    connect(ui->buttonFilterReset,
            &QPushButton::clicked,
            this,
            &PaneProfit::filterResetSkus);
    connect(ui->buttonRemoveSavedGroup,
            &QPushButton::clicked,
            this,
            &PaneProfit::removeGroup);
    connect(ui->buttonSaveNewGroup,
            &QPushButton::clicked,
            this,
            &PaneProfit::saveInNewGroup);
    connect(ui->buttonSaveSelGroup,
            &QPushButton::clicked,
            this,
            &PaneProfit::saveInCurrentGroup);
}
//----------------------------------------------------------
PaneProfit::~PaneProfit()
{
    delete ui;
}
//----------------------------------------------------------
void PaneProfit::compute()
{
    QString plainText = ui->plainTextEditSkus->toPlainText().trimmed();
    if (!plainText.isEmpty()) {
        QStringList skus = plainText.split("\n");
        QStringList headerVert;
        QStringList headerHoriz;
        QList<QList<QVariant>> elements;
        SkuFoundTableModel::instance()->fillTable(
                    headerVert,
                    headerHoriz,
                    elements,
                    skus,
                    ui->spinBoxUnitPrice->value(),
                    ui->spinBoxMOQ->value(),
                    ui->spinBoxMonthInventory->value(),
                    ui->spinBoxAdsCostPerSale->value());
        ui->tableWidgetProfit->clear();
        int nCols = headerHoriz.size();
        int nRows = headerVert.size();
        ui->tableWidgetProfit->setColumnCount(nCols);
        ui->tableWidgetProfit->setRowCount(nRows);
        ui->tableWidgetProfit->setHorizontalHeaderLabels(headerHoriz);
        ui->tableWidgetProfit->setVerticalHeaderLabels(headerVert);
        for (int i=0; i<nRows; ++i) {
            for (int j=0; j<nCols; ++j) {
                auto item = new QTableWidgetItem();
                item->setData(Qt::DisplayRole, elements[i][j]);
                ui->tableWidgetProfit->setItem(
                            i, j, item);
            }
        }
        QMap<QDate, int> chartSales;
        QMap<QDate, int> chartRefunds;
        SkuFoundTableModel::instance()->fillChartData(
                    chartSales, chartRefunds, skus);
        _displayChartData(chartSales, chartRefunds);
    }
}
//----------------------------------------------------------
void PaneProfit::_displayChartData(
        const QMap<QDate, int> &chartSales,
        const QMap<QDate, int> &chartRefunds)
{
    if (chartSales.size() > 2) {
        QChart *previousChart = ui->chartView->chart();
        QChart *chart = new QChart();
        //QChart *chart = new QChart();

        // Création des axes horizontaux et verticaux
        QDateTimeAxis *axisX = new QDateTimeAxis;
        axisX->setTickCount(10);
        axisX->setFormat("dd/MM/yyyy");
        axisX->setTitleText("Date");
        axisX->setRange(QDateTime(chartSales.firstKey(), QTime(0, 0, 0)),
                        QDateTime(chartSales.lastKey(), QTime(0, 0, 0)));
        chart->addAxis(axisX, Qt::AlignBottom);

        QValueAxis *axisY1 = nullptr;
        //QValueAxis *axisY2 = nullptr;

        QString labelSales("Ventes");
        QString labelRefunds("Remboursements");

        axisY1 = new QValueAxis;
        axisY1->setLabelFormat("%d");
        axisY1->setTitleText(labelSales);
        int maxSales = *std::max_element(
                    chartSales.begin(), chartSales.end());// * 1.2 + 1;
        //int maxRefunds = *std::max_element(
                    //chartRefunds.begin(), chartRefunds.end());// * 1.2 + 1;
        //double ratioRefund = double(maxRefunds) * maxSales;
        axisY1->setRange(0, maxSales); // * 1.2 + 1);
        chart->addAxis(axisY1, Qt::AlignLeft);

        // Ajout des courbes
        QLineSeries *series1 = new QLineSeries;
        QLineSeries *series2 = new QLineSeries;

        for (auto it = chartSales.begin(); it != chartSales.end(); ++it) {
            QDateTime date = QDateTime(it.key(), QTime(0, 0, 0));
            series1->append(date.toMSecsSinceEpoch(), it.value());
        }
        series1->setName(labelSales);
        series1->attachAxis(axisX);
        series1->attachAxis(axisY1);
        chart->addSeries(series1);

        for (auto it = chartRefunds.begin(); it != chartRefunds.end(); ++it) {
            QDateTime date = QDateTime(it.key(), QTime(0, 0, 0));
            series2->append(date.toMSecsSinceEpoch(), it.value());
        }
        /// Workaroud so series2 will have the same scale as series1. This dot is deleted later.
        QDateTime dateTimeMaxPlus1 = QDateTime(
                    chartRefunds.lastKey().addDays(1), QTime(0, 0, 0));
        series2->append(dateTimeMaxPlus1.toMSecsSinceEpoch(), maxSales);

        series2->setName(labelRefunds);
        series2->attachAxis(axisX);
        series2->attachAxis(axisY1);
        chart->addSeries(series2);

        chart->setTitle(tr("Ventes et remboursements"));
        chart->legend()->setVisible(true);
        chart->legend()->setAlignment(Qt::AlignBottom);
        ui->chartView->setChart(chart);
        series2->remove(dateTimeMaxPlus1.toMSecsSinceEpoch(), maxSales);

        ui->chartView->setRenderHint(QPainter::Antialiasing);
        ui->chartView->setMinimumSize(800, 600);
        if (previousChart != nullptr) {
            previousChart->deleteLater();
        }
    }
}
//----------------------------------------------------------
void PaneProfit::loadYearSkus()
{
    setCursor(Qt::WaitCursor);
    int year = ui->comboBoxYear->currentText().toInt();
    if (QDate::currentDate().month() < 3) {
        --year; // TODO VERSION3 it sucks that it is only by end of year
    }
    ///* /// To load faster, I can remove this
    ///
    VatOrdersModel::instance()->computeVatMinimalyEcom(
                {year-1, year}
                //, [](const Shipment *) {}
                , [](const Shipment *shipment) {
        SkuFoundTableModel::instance()->addShipmentOrRefund(
                    shipment);
    }
    , [](const Shipment *) -> bool {return true;}
    );
    SkuFoundTableModel::instance()->compute();
    setCursor(Qt::ArrowCursor);
}
//----------------------------------------------------------
void PaneProfit::addSku()
{
    auto selIndexes = ui->tableKnownSkus->selectionModel()->selectedIndexes();
    if (selIndexes.size() > 0) {
        QString plainText = ui->plainTextEditSkus->toPlainText().trimmed();
        QStringList lines = plainText.split("\n");
        for (auto itIndex = selIndexes.rbegin();
             itIndex != selIndexes.rend(); ++itIndex) {
            if (itIndex->column() == 0 && !ui->tableKnownSkus->isRowHidden(itIndex->row())) {
                QString sku = itIndex->data().toString();
                if (!lines.contains(sku, Qt::CaseInsensitive)) {
                    lines.insert(0, sku);
                }
            }
        }
        ui->plainTextEditSkus->setPlainText(lines.join("\n"));
    }
}
//----------------------------------------------------------
void PaneProfit::filterSkus()
{
    QString filteredText = ui->lineEditFilterSkus->text();
    if (!filteredText.isEmpty()) {
        int nRows = SkuFoundTableModel::instance()->rowCount();
        for (int i=0; i<nRows; ++i) {
            bool contains = SkuFoundTableModel::instance()->
                    containsText(i, filteredText);
            ui->tableKnownSkus->setRowHidden(i, !contains);
        }
    }
}
//----------------------------------------------------------
void PaneProfit::filterResetSkus()
{
    int nRows = SkuFoundTableModel::instance()->rowCount();
    for (int i=0; i<nRows; ++i) {
        ui->tableKnownSkus->setRowHidden(i, false);
    }
}
//----------------------------------------------------------
void PaneProfit::removeGroup()
{
    auto selIndexes = ui->listViewGroups->selectionModel()->selectedIndexes();
    if (selIndexes.size() > 0) {
        SkusGroupProfitModel::instance()
                ->removeGroup(selIndexes.first());
    }
}
//----------------------------------------------------------
void PaneProfit::saveInCurrentGroup()
{
    auto selIndexes = ui->listViewGroups->selectionModel()->selectedIndexes();
    if (selIndexes.size() > 0) {
        SkusGroupProfitModel::instance()
                ->setTextOfGroup(
                    selIndexes.first(),
                    ui->plainTextEditSkus->toPlainText());
    }
}
//----------------------------------------------------------
void PaneProfit::saveInNewGroup()
{
    QString groupName = QInputDialog::getText(
                this,
                tr("Nom du groupe"),
                tr("Entrez le nom du groupe de SKU"));
    if (!groupName.isEmpty()) {
        QString idGroup = SkusGroupProfitModel::instance()
                ->addGroup(groupName);
        SkusGroupProfitModel::instance()
                ->setTextOfGroup(
                    idGroup,
                    ui->plainTextEditSkus->toPlainText());
        ui->listViewGroups->selectionModel()
                ->select(
                    SkusGroupProfitModel::instance()->index(
                        SkusGroupProfitModel::instance()->rowCount()-1, 0),
                    QItemSelectionModel::Select);
    }
}
//----------------------------------------------------------
void PaneProfit::_onGroupSelected(
        const QItemSelection &selected,
        const QItemSelection &unselected)
{
    if (selected.size() > 0) {
        auto index = selected.indexes().first();
        QString text = SkusGroupProfitModel::instance()
                ->getTextOfGroup(index);
        ui->plainTextEditSkus->setPlainText(text);
    } else if (unselected.size() > 0) {
        ui->plainTextEditSkus->clear();
    }
}
//----------------------------------------------------------
