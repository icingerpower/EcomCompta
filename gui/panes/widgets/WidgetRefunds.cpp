#include <qmessagebox.h>
#include <QtCore/qsettings.h>
#include <QtCore/qfile.h>
#include <QtCore/qtextstream.h>
#include <qfiledialog.h>

#include "WidgetRefunds.h"
#include "ui_WidgetRefunds.h"
#include "model/orderimporters/OrderManager.h"
#include "model/orderimporters/RefundManager.h"
#include "model/SettingManager.h"

//----------------------------------------------------------
WidgetRefunds::WidgetRefunds(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WidgetRefunds)
{
    ui->setupUi(this);
    connect(ui->buttonRemove,
            &QPushButton::clicked,
            this,
            &WidgetRefunds::removeSelectedRefunds);
    connect(ui->buttonExpandAll,
            &QPushButton::clicked,
            this,
            &WidgetRefunds::expandAll);
    connect(ui->buttonExport,
            &QPushButton::clicked,
            this,
            &WidgetRefunds::exportCsv);
    connect(ui->buttonChangeDate,
            &QPushButton::clicked,
            this,
            &WidgetRefunds::changeDate);
}
//----------------------------------------------------------
WidgetRefunds::~WidgetRefunds()
{
    delete ui;
}
//----------------------------------------------------------
void WidgetRefunds::init(
        OrderManager *orderManager, RefundManager *refundManager)
{
    m_orderManager = orderManager;
    m_refundManager = refundManager;
    ui->treeViewRefunds->setModel(m_refundManager);
    ui->treeViewRefunds->header()->resizeSection(0, 200);
}
//----------------------------------------------------------
void WidgetRefunds::clear()
{
    ui->treeViewRefunds->setModel(nullptr);
}
//----------------------------------------------------------
int WidgetRefunds::getRefundCount() const
{
    auto model = ui->treeViewRefunds->model();
    if (model != nullptr) {
        return model->rowCount();
    }
    return 0;
}
//----------------------------------------------------------
void WidgetRefunds::hideVatColumns()
{
    auto model = ui->treeViewRefunds->model();
    if (model != nullptr && m_refundManager != nullptr) {
        auto indexes = m_refundManager->vatColIndexes();
        for (auto index = indexes.rbegin(); index != indexes.rend(); ++index) {
            ui->treeViewRefunds->hideColumn(*index);
        }
    }
}
//----------------------------------------------------------
void WidgetRefunds::removeSelectedRefunds()
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
                ui->treeViewRefunds->expandAll();
            }
        } catch (const RefundIdException &e) {
            QMessageBox::critical(
                        this, tr("Erreur"),
                        e.error() + "\n" + tr("La suppression a été annulée car il est uniquement possible de supprimer les remboursements manuels"),
                        QMessageBox::Ok);
        }
    }
}
//----------------------------------------------------------
void WidgetRefunds::expandAll()
{
    ui->treeViewRefunds->expandAll();
}
//----------------------------------------------------------
void WidgetRefunds::exportCsv()
{
    QSettings settings;
    QString key = "WidgetRefunds__exportCsv";
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
        int nCols = m_refundManager->columnCount();
        QStringList elements;
        for (int i=0; i<nCols; ++i) {
            elements << m_refundManager->headerData(i, Qt::Horizontal).toString();
        }
        QString sep = ";";
        lines << elements.join(sep);
        int nRows = m_refundManager->rowCount();
        for (int i = 0; i<nRows; ++i) {
            QModelIndex indexImporter = m_refundManager->index(i, 0, QModelIndex());
            for (int j = 0; j < m_refundManager->rowCount(indexImporter); ++j) {
                QModelIndex indexYear = m_refundManager->index(j, 0, indexImporter);
                for (int k = 0; k < m_refundManager->rowCount(indexYear); ++k) {
                    bool hidden = ui->treeViewRefunds->isRowHidden(k, indexYear);
                    if (!hidden) {
                        elements.clear();
                        for (int l=0; l<nCols; ++l) {
                            auto index = ui->treeViewRefunds->model()->index(k, l, indexYear);
                            elements << ui->treeViewRefunds->model()->data(index).toString();
                        }
                        lines << elements.join(sep);
                    }
                }
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
void WidgetRefunds::changeDate()
{
    //TODO
}
//----------------------------------------------------------

