#include <qmessagebox.h>
#include <qinputdialog.h>
#include <qfiledialog.h>
#include <QtCore/qsettings.h>

#include "../common/utils/CsvHeader.h"

#include "PaneCustomOrderParams.h"
#include "ui_PaneCustomOrderParams.h"

#include "model/orderimporters/OrderImporterCustomManager.h"
#include "model/orderimporters/OrderMapping.h"
#include "model/orderimporters/OrderImporterCustomParams.h"
#include "model/orderimporters/OrderImporterCustom.h"
#include "model/orderimporters/OrderImporterCustomException.h"

//----------------------------------------------------------
PaneCustomOrderParams::PaneCustomOrderParams(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PaneCustomOrderParams)
{
    ui->setupUi(this);
    ui->listViewCustoms->setModel(
                OrderImporterCustomManager::instance());
    connect(ui->buttonAdd,
            &QPushButton::clicked,
            this,
            &PaneCustomOrderParams::add);
    connect(ui->buttonRemove,
            &QPushButton::clicked,
            this,
            &PaneCustomOrderParams::remove);
    connect(ui->buttonTest,
            &QPushButton::clicked,
            this,
            &PaneCustomOrderParams::test);
    connect(ui->listViewCustoms->selectionModel(),
            &QItemSelectionModel::selectionChanged,
            [this](const QItemSelection &selected, const QItemSelection &){
        if (selected.size() > 0) {
            int index = selected.indexes()[0].row();
            ui->tableViewSelectedCustoms->setModel(
                        OrderImporterCustomManager::instance()->customParams(index));
            ui->tableViewSelectedCustoms->horizontalHeader()->resizeSection(0, 250);
            ui->tableViewSelectedCustoms->horizontalHeader()->resizeSection(1, 250);
        } else {
            ui->tableViewSelectedCustoms->setModel(nullptr);
        }
    });
    ui->listViewCustoms->setCurrentIndex(
                OrderImporterCustomManager::instance()
                ->index(0, 0));
}
//----------------------------------------------------------
PaneCustomOrderParams::~PaneCustomOrderParams()
{
    delete ui;
}
//----------------------------------------------------------
void PaneCustomOrderParams::add()
{
    QString text = QInputDialog::getText(
                this, tr("Nom"),
                tr("Entrez le nom"));
    if (!text.isEmpty()) {
        OrderImporterCustomManager::instance()->addCustomParams(
                    text);
    }
}
//----------------------------------------------------------
void PaneCustomOrderParams::remove()
{
    auto rows = ui->listViewCustoms->selectionModel()->selectedRows();
    if (rows.size() > 0) {
        ui->tableViewSelectedCustoms->setModel(nullptr);
        OrderImporterCustomManager::instance()->removeCustomParams(
                    rows[0].row());
    }
}
//----------------------------------------------------------
void PaneCustomOrderParams::test()
{
    auto rows = ui->listViewCustoms->selectionModel()->selectedRows();
    if (rows.size() > 0) {
        int index = rows[0].row();
        QString importerId
                = OrderImporterCustomManager::instance()
                ->customParamsId(index);
        auto importer = OrderImporterCustomManager::instance()
                ->importer(importerId);
        QSettings settings;
        QString settingKey = "PaneCustomOrderParams__test";
        QString lastDirPath = settings.value(
                    settingKey, QDir().absolutePath()).toString();
        //QStringList extentions = importer->reportTypes()[0].extensions;
        QString filePath = QFileDialog::getOpenFileName(
                    this, tr("Choisir un fichier"),
                    lastDirPath, "CSV (*.csv)");
        if (!filePath.isEmpty()) {
            settings.setValue(settingKey, QFileInfo(filePath).path());
            try {
                auto importMapping = importer->loadReport(
                            OrderImporterCustom::REPORT_TYPE,
                            filePath,
                            QDate::currentDate().year());
                int nOrders = importMapping->orderById.size();
                if (nOrders == 0) {
                    QMessageBox::critical(
                                this, tr("Fichié importé sans commandes"),
                                tr("Aucunes commandes n'a été trouvé dans le fichier."),
                                QMessageBox::Ok);
                } else {
                    QMessageBox::information(
                                this, tr("Fichié importé"),
                                tr("Le fichier a été importé correctement avec") + " "
                                + QString::number(nOrders) + " " + tr("commandes"),
                                QMessageBox::Ok);
                }
            } catch(const OrderImporterCustomException &exception) {
                QMessageBox::critical(
                            this, tr("Erreur dans les colonnes indiqués"),
                            exception.error(),
                            QMessageBox::Ok);
            } catch(const CsvHeaderException &exception) {
                QMessageBox::critical(
                            this, tr("Erreur dans l'entête"),
                            tr("L'entête du fichier ne contient pas l'une de ces colonnes ")
                            + exception.columnValuesError().join(" ,"),
                            QMessageBox::Ok);
            } catch(const CsvCustomerHeaderException &exception) {
                QMessageBox::critical(
                            this, tr("Erreur dans l'entête"),
                            exception.error(),
                            QMessageBox::Ok);
            } catch(const OrderImporterException &exception) {
                QMessageBox::critical(
                            this, tr("Erreur"),
                            exception.error(),
                            QMessageBox::Ok);
            }
        }
    }
}
//----------------------------------------------------------
