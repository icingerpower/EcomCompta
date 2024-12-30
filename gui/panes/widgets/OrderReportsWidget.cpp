#include <qmessagebox.h>
#include <QtCore/qtimer.h>
#include <QtCharts/qlineseries.h>
#include <QtCharts/qdatetimeaxis.h>
#include <QtCharts/qvalueaxis.h>
#include <QtCharts/qchart.h>
#include <QtCore/qsettings.h>
#include <QtCore/qdatetime.h>
#include <QtCore/qfile.h>
#include <QtCore/qhash.h>
#include <QFileDialog>
#include <qglobal.h>
#include <qdebug.h>

#include "OrderReportsWidget.h"
#include "ui_OrderReportsWidget.h"
#include "model/orderimporters/AbstractOrderImporter.h"
#include "model/orderimporters/OrderManager.h"
#include "model/orderimporters/RefundManager.h"
#include "model/CustomerManager.h"
#include "model/SettingManager.h"
//#include "model/utils/CsvHeader.h"
#include "../common/utils/CsvHeader.h"
#include "model/utils/FileSystemModelNoDir.h"
#include "model/orderimporters/ImportedFileReportManager.h"
#include "model/ChangeNotifier.h"


//----------------------------------------------------------
OrderReportsWidget::OrderReportsWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::OrderReportsWidget)
{
    ui->setupUi(this);
    m_importer = nullptr;
    m_fileModel = nullptr;
    m_orderManager = nullptr;
    _showOrHideOrderRefundTabs();
    _connectSlots();
}
//----------------------------------------------------------
void OrderReportsWidget::_connectSlots()
{
    connect(ui->buttonImportReports,
            &QPushButton::clicked,
            this,
            &OrderReportsWidget::importReports);
    connect(ui->buttonRemoveReports,
            &QPushButton::clicked,
            this,
            &OrderReportsWidget::removeSelectedReports);
    connect(ui->buttonLoadDataExisting,
            &QPushButton::clicked,
            this,
            &OrderReportsWidget::loadSelectedReports);
    connect(ui->buttonLoadAll,
            &QPushButton::clicked,
            this,
            &OrderReportsWidget::loadAllReports);
}
//----------------------------------------------------------
void OrderReportsWidget::_showOrHideOrderRefundTabs()
{
    int nRefunds = ui->tabRefunds->getRefundCount();
    if (nRefunds > 0) {
        ui->tabWidget->setTabVisible(2, true);
        ui->tabRefunds->hideVatColumns();
    } else {
        //ui->tabWidget->setTabEnabled(1, false);
        ui->tabWidget->setTabVisible(2, false);
    }
    int nOrders = ui->tabOrders->getOrderCount();
    if (nOrders > 0) {
        ui->tabWidget->setTabVisible(1, true);
        ui->tabOrders->hideVatColumns();
    } else {
        //ui->tabWidget->setTabEnabled(1, false);
        ui->tabWidget->setTabVisible(1, false);
    }
}
//----------------------------------------------------------
OrderReportsWidget::~OrderReportsWidget()
{
    delete ui;
    m_refundManager->deleteLater();
}
//----------------------------------------------------------
void OrderReportsWidget::init(
        const ReportType &reportType,
        AbstractOrderImporter *importer)
{
    m_reportType = reportType;
    m_importer = importer;
    m_orderManager = OrderManager::instance()->copyEmpty();
    m_refundManager = new RefundManager(m_orderManager);

    //ui->treeViewRefunds->setModel(m_refundManager);
    //ui->treeViewRefunds->header()->resizeSection(0, 200);
    ui->tabOrders->init(m_orderManager, m_refundManager);
    ui->tabRefunds->init(m_orderManager, m_refundManager);
    ui->groupBoxMain->setTitle(reportType.shortName);
    connect(ui->buttonHelp,
            &QPushButton::clicked,
            [this, reportType](bool){
        QMessageBox::information(
                    this, tr("Informations sur le rapport"),
                    reportType.helpText,
                    QMessageBox::Ok);
    });
    QDir workingDirectory = SettingManager::instance()->workingDirectory();
    m_fileModel = new FileSystemModelNoDir(this);
    m_fileModel->setReadOnly(false);
    ui->treeViewFolder->setModel(m_fileModel);
    connect(m_fileModel,
            &FileSystemModelNoDir::fileRenamed,
            ImportedFileReportManager::instance(),
            &ImportedFileReportManager::onFileRenamed);
    onCustomerSelectedChanged(
                CustomerManager::instance()->getSelectedCustomerId());
}
//----------------------------------------------------------
void OrderReportsWidget::onCustomerSelectedChanged(
        const QString &customerId)
{
    QDir reportDir = SettingManager::instance()->reportDirectory(
                m_importer->name(), m_reportType.shortName);
    Q_ASSERT(reportDir.exists());
    qInfo() << "Tried to create directory" << reportDir.absolutePath();
    m_fileModel->setRootPath(reportDir.absolutePath());
    ui->treeViewFolder->setRootIndex(m_fileModel->index(reportDir.absolutePath()));
    ui->treeViewFolder->header()->resizeSection(0, 280);
    ui->treeViewFolder->expandAll();
    QTimer::singleShot(1000, this, [this]{
        ui->treeViewFolder->expandAll();
    });
}
//----------------------------------------------------------
QString OrderReportsWidget::uniqueId() const
{
    return "OrderReportsWidget";
}
//----------------------------------------------------------
/*
QString OrderReportsWidget::reportDirPathRel() const
{
    QString reportPathRel = "reports";
    reportPathRel += QDir::separator() + m_importer->name();
    reportPathRel += QDir::separator() + m_reportType.shortName;
    return reportPathRel;
}
//----------------------------------------------------------
QDir OrderReportsWidget::reportDir() const
{
    QString reportPathRel = reportDirPathRel();
    QDir reportDir = SettingManager::instance()->workingDirectory();
    reportDir.cd(reportPathRel);
    return reportDir;
}
//*/
//----------------------------------------------------------
void OrderReportsWidget::importReports()
{
    QSettings settings;
    /*
    QString settingKey = "OrderReportsWidget::importReports-"
            + CustomerManager::instance()->getSelectedCustomerId()
            + m_reportType.shortName;
            //*/
    QDir lastDirectory;
    auto key = settingKey() + "-";
    key += "-" + m_reportType.shortName;
    if (settings.contains(key)) {
        lastDirectory = QDir(settings.value(key).toString());
    }
    QString fileFiler = "Report (*";
    fileFiler += m_reportType.extensions.join(" *.");
    fileFiler += ")";
    QStringList fileNames
            = QFileDialog::getOpenFileNames(
                this, "Fichiers",
                lastDirectory.absolutePath(),
                fileFiler);
    setCursor(Qt::WaitCursor);
    if (fileNames.size() > 0) {
        setCursor(Qt::WaitCursor);
        lastDirectory = QFileInfo(fileNames[0]).dir();
        settings.setValue(key, lastDirectory.absolutePath());
        QStringList fileNameExisting;
        QStringList fileContentExisting;
        QMap<int, QString> existingContentMappingErrors;
        QDir reportDir = SettingManager::instance()->reportDirectory(
                    m_importer->name(), m_reportType.shortName);
        QStringList relImportedFileNames = reportDir.entryList(QDir::Files);
        int id = 0;
        for (auto fileName : fileNames) {
            QFileInfo fileInfo(fileName);
            QString relFileName = fileInfo.fileName();
            QFile file(fileName);
            file.open(QFile::ReadOnly);
            auto content = file.readAll();
            file.close();
            for (auto relImportedFileName : relImportedFileNames) {
                QString importedFileName = reportDir.filePath(relImportedFileName);
                QFileInfo importedFileInfo(importedFileName);
                if (relFileName == relImportedFileName) {
                    fileNameExisting << relFileName;
                    continue;
                }
                QFile importedFile(importedFileName);
                importedFile.open(QFile::ReadOnly);
                auto importedContent = importedFile.readAll();
                importedFile.close();
                if (content == importedContent) {
                    fileContentExisting << relFileName;
                    existingContentMappingErrors[id] = relFileName + " => " + relImportedFileName;
                }
            }
            ++id;
        }
        if (fileNameExisting.size() > 0) {
            int ret = QMessageBox::warning(
                        this, tr("Nom de fichier existant"),
                        tr("Êtes-vous sûr de vouloir remplacer les fichiers existants ?\n")
                        + fileNameExisting.join("\n") +
                        tr("\nCette action n'est pas réversible."),
                        QMessageBox::Ok | QMessageBox::Cancel);
            if (ret != QMessageBox::Ok) {
                return;
            }
        }
        if (fileContentExisting.size() > 0) {
            QStringList elements;
            int ret = QMessageBox::warning(
                        this, tr("Contenu de fichier existant"),
                        "Certains rapports ont déjà été importés sous un autre nom et seront ignorés.\n"
                        "Êtes-vous d'accord?.\n"
                        + existingContentMappingErrors.values().join("\n"),
                        QMessageBox::Ok | QMessageBox::Cancel);
            if (ret == QMessageBox::Ok) {
                for (auto it = existingContentMappingErrors.begin();
                     it != existingContentMappingErrors.end();
                     ++it) {
                    fileNames.removeAt(it.key());
                }
            } else {
                return;
            }
        }
        if (fileNames.size() > 0) {
            displayFileNames(fileNames, true);
            ChangeNotifier::instance()->emitDataForVatChanged();
        } else {
            QMessageBox::information(
                        this, tr("Information"),
                        "Aucun fichier n'a été importé.",
                        QMessageBox::Ok);
        }
        ui->treeViewFolder->expandAll();
    }
    setCursor(Qt::ArrowCursor);
}
//----------------------------------------------------------
void OrderReportsWidget::displayFileNames(
        const QStringList &fileNames, bool copyFilesInDir)
{
    if (fileNames.size() > 0) {
        setCursor(Qt::WaitCursor);
        OrdersMapping allOrders;
        try {
            QHash<QString, QPair<QDate, QDate>> fileDates;
            for (auto fileName : fileNames) {
                auto ordersMapping = m_importer->loadReport(
                            m_reportType.shortName,
                            fileName,
                            QDate::currentDate().year());
                fileDates[fileName] = {ordersMapping->minDate.date(), ordersMapping->maxDate.date()};
                allOrders.uniteFromSameKindOfReport(*ordersMapping.data());
                m_refundManager->retriveRefunds();
            }
            if (copyFilesInDir) {
                QDir reportDir = SettingManager::instance()->reportDirectory(
                            m_importer->name(), m_reportType.shortName);
                for (auto fileName : fileNames) {
                    QFileInfo fileInfo(fileName);
                    QDir reportDirYear = reportDir;
                    QString dirYearName = QString::number(fileDates[fileName].first.year());
                    if (fileDates[fileName].first.year() != fileDates[fileName].second.year()) {
                        dirYearName += "-" + QString::number(fileDates[fileName].second.year());
                    }
                    reportDirYear.mkpath(dirYearName);
                    reportDirYear.cd(dirYearName);
                    QString relFileName = fileInfo.fileName();
                    QString newFilePath = reportDirYear.filePath(relFileName);
                    QFile file(fileName);
                    ImportedFileReportManager::instance()
                            ->recordFile(
                                fileName,
                                m_importer->name(),
                                m_reportType.shortName,
                                fileDates[fileName].first,
                                fileDates[fileName].second);
                    file.copy(fileName, newFilePath);
                    ui->treeViewFolder->expandAll();
                }
            }
            QtCharts::QDateTimeAxis *axisX = new QtCharts::QDateTimeAxis;
            axisX->setTickCount(10);
            axisX->setFormat("dd/MM/yy");
            QFont fontX = axisX->labelsFont();
            fontX.setPixelSize(16);
            axisX->setLabelsFont(fontX);

            QtCharts::QValueAxis *axisY = new QtCharts::QValueAxis;
            axisY->setLabelFormat("%i");
            QFont fontY = axisY->labelsFont();
            fontY.setPixelSize(16);
            axisY->setLabelsFont(fontY);

            QtCharts::QChart *previousChart = ui->chartView->chart();
            QtCharts::QChart *chart = new QtCharts::QChart();
            chart->addAxis(axisX, Qt::AlignBottom);
            chart->addAxis(axisY, Qt::AlignLeft);
            int maxY = 1;
            QDateTime maxX = QDateTime(QDate(2000,1,1));
            QDateTime minX = QDateTime::currentDateTime();
            QHash<QString, QtCharts::QLineSeries *> allSeries;
            for (auto itYear = allOrders.ordersQuantityByDate.begin();
                 itYear != allOrders.ordersQuantityByDate.end();
                 ++itYear) {
                for (auto itChannel = itYear.value().begin();
                     itChannel != itYear.value().end();
                     ++itChannel) {
                    QtCharts::QLineSeries *series;
                    if (!allSeries.contains(itChannel.key())) {
                        series = new QtCharts::QLineSeries(ui->chartView);
                        series->setName(itChannel.key());
                        allSeries[itChannel.key()] = series;
                    }
                    series = allSeries[itChannel.key()];
                    //QtCharts::QLineSeries *series
                    QDateTime last = QDateTime(QDate(2000,1,1));
                    for (auto it = itChannel.value().begin();
                         it != itChannel.value().end();
                         ++it) {
                        QDateTime dateTime(it.key());
                        Q_ASSERT(last < dateTime);
                        last = dateTime;
                        series->append(dateTime.toMSecsSinceEpoch(), it.value());
                        maxY = qMax(it.value(), maxY);
                        maxX = qMax(dateTime, maxX);
                        minX = qMin(dateTime, minX);
                    }
                    //chart->addSeries(series);
                    //series->attachAxis(axisX);
                    //series->attachAxis(axisY);
                }
            }
            for (auto series : allSeries) {
                chart->addSeries(series);
                series->attachAxis(axisX);
                series->attachAxis(axisY);
            }
            axisX->setMin(minX);
            axisX->setMax(maxX);
            axisY->setMax(maxY);

            ui->chartView->setRenderHint(QPainter::Antialiasing);
            ui->chartView->setChart(chart);
            if (previousChart != nullptr) {
                delete previousChart;
            }
            ui->chartView->setRubberBand(QtCharts::QChartView::HorizontalRubberBand);
            m_orderManager->clearOrders();
            m_orderManager->recordOrders(m_importer->name(), allOrders);
            m_orderManager->createRefundsFromUncomplete();
            ui->tabOrders->expandAllOrders();
            ui->tabRefunds->expandAll();
            //ui->treeViewRefunds->expandAll();
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
        setCursor(Qt::ArrowCursor);
        _showOrHideOrderRefundTabs();
    }
}
//----------------------------------------------------------
void OrderReportsWidget::loadSelectedReports()
{
    auto selection = ui->treeViewFolder->selectionModel()->selectedRows();
    if (selection.size() > 0) {
        QDir reportDir = SettingManager::instance()->reportDirectory(
                    m_importer->name(), m_reportType.shortName);
        QStringList fileNames;
        for (auto row : selection) {
            QString yearDirName = row.parent().data().toString();
            QString relFileName = row.data().toString();
            QString fileName = reportDir.filePath(
                        yearDirName + QDir::separator() + relFileName);
            fileNames << fileName;
        }
        displayFileNames(fileNames, false);
    }
}
//----------------------------------------------------------
void OrderReportsWidget::loadAllReports()
{
    QModelIndex rootIndex = ui->treeViewFolder->rootIndex();
    int nRows = ui->treeViewFolder->model()->rowCount(rootIndex);
    if (nRows > 0) {
        QDir reportDir = SettingManager::instance()->reportDirectory(
                    m_importer->name(), m_reportType.shortName);
        QStringList fileNames;
        for (int i=0; i<nRows; ++i) {
            auto yearIndex = m_fileModel->index(i, 0, rootIndex);
            int nRowFiles = m_fileModel->rowCount(yearIndex);
            QString yearDirName = yearIndex.data().toString();
            for (int j=0; j<nRowFiles; ++j) {
                auto fileIndex = m_fileModel->index(j, 0, yearIndex);
                QString relFileName = fileIndex.data().toString();
                QString fileName = reportDir.filePath(
                        yearDirName + QDir::separator() + relFileName);
                fileNames << fileName;
            }
        }
        displayFileNames(fileNames, false);
    }
}
//----------------------------------------------------------
void OrderReportsWidget::removeSelectedReports()
{
    auto selection = ui->treeViewFolder->selectionModel()->selectedRows();
    if (selection.size() > 0) {
        int ret = QMessageBox::warning(
                    this, tr("Suppression des rapports"),
                    tr("Êtes-vous sûr de vouloir supprimer les rapports sélectionnés ?\n"
                       "Cette action n'est pas réversible."),
                    QMessageBox::Ok | QMessageBox::Cancel);
        if (ret == QMessageBox::Ok) {
            QDir reportDir = SettingManager::instance()->reportDirectory(
                        m_importer->name(), m_reportType.shortName);
            QHash<QString, QDir> reportDirYears;
            while (selection.size() > 0) {
                auto lastSel = selection.takeLast();
                QString dirYear = lastSel.parent().data().toString();
                QDir reportDirYear = reportDir.filePath(dirYear);
                reportDirYears[dirYear] = reportDirYear;
                QString fileName = lastSel.data().toString();
                reportDirYear.remove(fileName);
                ImportedFileReportManager::instance()->removeFile(
                            reportDirYear.filePath(fileName),
                            m_importer->name(),
                            m_reportType.shortName);
            }
            for (auto itDir = reportDirYears.begin();
                 itDir != reportDirYears.end();
                 ++itDir) {
                if (itDir.value().entryList(QDir::Files).size() == 0) {
                    itDir.value().removeRecursively();
                }
            }
        }
    }
}

//----------------------------------------------------------
/*
void OrderReportsWidget::removeSelectedRefunds()
{
    auto selection = ui->treeViewRefunds->selectionModel()->selectedRows();
    if (selection.size() > 0) {
        try {
            int ret = QMessageBox::warning(
                        this, tr("Suppression des remboursements"),
                        tr("Êtes-vous sûr de vouloir supprimer les remboursements sélectionnés ?") + "\n"
                        + tr("Cette action n'est pas réversible."),
                        QMessageBox::Ok | QMessageBox::Cancel);
            if (ret == QMessageBox::Ok) {
                while (selection.size() > 0) {
                    auto sel = selection.takeFirst();
                    m_refundManager->removeRefund(sel);
                }
            }
        } catch (const RefundIdException &e) {
            QMessageBox::critical(
                        this, tr("Erreur"),
                        e.error() + "\n" + tr("La suppression a été annulée car il est uniquement possible de supprimer les remboursements manuels"),
                        QMessageBox::Ok);
        }
    }

}
//*/
//----------------------------------------------------------

