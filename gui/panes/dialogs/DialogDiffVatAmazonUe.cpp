#include <QtCore/qsettings.h>
#include <QtCore/qfile.h>
#include <QtCore/qtextstream.h>
#include <qfiledialog.h>

#include "DialogDiffVatAmazonUe.h"
#include "ui_DialogDiffVatAmazonUe.h"

#include "model/orderimporters/ModelDiffAmazonUE.h"
#include "model/orderimporters/Shipment.h"
#include "model/SettingManager.h"

//----------------------------------------------------------
DialogDiffVatAmazonUe::DialogDiffVatAmazonUe(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogDiffVatAmazonUe)
{
    ui->setupUi(this);
    connect(ui->buttonFilter,
            &QPushButton::clicked,
            this,
            &DialogDiffVatAmazonUe::filter);
    connect(ui->buttonResetFilter,
            &QPushButton::clicked,
            this,
            &DialogDiffVatAmazonUe::resetFilter);
    connect(ui->buttonExport,
            &QPushButton::clicked,
            this,
            &DialogDiffVatAmazonUe::exportOrders);
}
//----------------------------------------------------------
DialogDiffVatAmazonUe::~DialogDiffVatAmazonUe()
{
    delete ui;
}
//----------------------------------------------------------
void DialogDiffVatAmazonUe::setModelDiffAmazonUE(ModelDiffAmazonUE *model)
{
    ui->tableViewDiff->setModel(model);
    ui->tableViewDiff->horizontalHeader()->resizeSection(0, 120);
    ui->tableViewDiff->horizontalHeader()->resizeSection(2, 150);
    ui->tableViewDiff->horizontalHeader()->resizeSection(3, 200);
    ui->tableViewDiff->horizontalHeader()->resizeSection(model->columnCount()-1, 300);
    ui->labelDiff->setText(QString::number(model->rowCount()));
    ui->labelTotal->setText(QString::number(model->totalShipmentAnalyzed()));
}
//----------------------------------------------------------
void DialogDiffVatAmazonUe::filter()
{
    QString commandeNumber = ui->lineEditOrderId->text().trimmed();
    int nRows = ui->tableViewDiff->model()->rowCount();
    for (int i=0; i<nRows; ++i) {
        auto indexOrder = ui->tableViewDiff->model()->index(
                    i, 2);
        auto indexRegime = ui->tableViewDiff->model()->index(
                    i, 7);
        auto indexRegimeAmz = ui->tableViewDiff->model()->index(
                    i, 8);
        bool show = ui->tableViewDiff->model()->data(indexOrder).toString().contains(commandeNumber);
        if (ui->radioNormal->isChecked()) {
            show &= (ui->tableViewDiff->model()->data(indexRegimeAmz).toString() == "REGULAR"
                    || ui->tableViewDiff->model()->data(indexRegime).toString() == Shipment::VAT_REGIME_NONE
                    || ui->tableViewDiff->model()->data(indexRegime).toString() == Shipment::VAT_REGIME_NORMAL
                    || ui->tableViewDiff->model()->data(indexRegime).toString() == Shipment::VAT_REGIME_NORMAL_EXPORT);
        } else if (ui->radioOss->isChecked()) {
            show &= (ui->tableViewDiff->model()->data(indexRegimeAmz).toString() == "UNION-OSS"
                     || ui->tableViewDiff->model()->data(indexRegime).toString() == Shipment::VAT_REGIME_OSS);
        } else if (ui->radioIoss->isChecked()) {
            show &= (ui->tableViewDiff->model()->data(indexRegimeAmz).toString() == "DEEMED_RESELLER-IOSS"
                     || ui->tableViewDiff->model()->data(indexRegime).toString() == Shipment::VAT_REGIME_IOSS);
        }
        ui->tableViewDiff->setRowHidden(i, !show);
    }
}
//----------------------------------------------------------
void DialogDiffVatAmazonUe::exportOrders()
{
    QSettings settings;
    QString key = "DialogDiffVatAmazonUe__exportOrders";
    QString lastDirPath = settings.value(
                key, QDir().absolutePath()).toString();
    QString filePath = QFileDialog::getSaveFileName(
                this, tr("Choisir un fichier"),
                lastDirPath,
                "CSV (*.csv)");
    if (!filePath.isEmpty()) {
        if (!filePath.toLower().endsWith(".csv")) {
            filePath += ".csv";
        }
        QFileInfo fileInfo(filePath);
        settings.setValue(key, fileInfo.absoluteDir().absolutePath());
        QStringList lines;
        int nCols = ui->tableViewDiff->model()->columnCount();
        QStringList elements;
        for (int i=0; i<nCols; ++i) {
            elements << ui->tableViewDiff->model()->headerData(i, Qt::Horizontal).toString();
        }
        QString sep = ";";
        lines << elements.join(sep);
        int nRows = ui->tableViewDiff->model()->rowCount();
        for (int i = 0; i<nRows; ++i) {
            bool hidden = ui->tableViewDiff->isRowHidden(i);
            if (!hidden) {
                elements.clear();
                for (int j=0; j<nCols; ++j) {
                    auto index = ui->tableViewDiff->model()->index(i, j);
                    elements << ui->tableViewDiff->model()->data(index).toString();
                }
                lines << elements.join(sep);
            }
        }
        QFile file(filePath);
        if (file.open(QFile::WriteOnly)) {
            QString fileContent = lines.join(SettingManager::instance()->returnLine());
            QTextStream stream(&file);
            stream << fileContent;
            file.close();
        }
    }
}
//----------------------------------------------------------
void DialogDiffVatAmazonUe::resetFilter()
{
    int nRows = ui->tableViewDiff->model()->rowCount();
    for (int i=0; i<nRows; ++i) {
        ui->tableViewDiff->setRowHidden(i, false);
    }
}
//----------------------------------------------------------
