#include <QtCore/qdebug.h>

#include "PaneImportedReportFiles.h"
#include "ui_PaneImportedReportFiles.h"
#include "model/orderimporters/ImportedFileReportManager.h"

//----------------------------------------------------------
PaneImportedReportFiles::PaneImportedReportFiles(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PaneImportedReportFiles)
{
    ui->setupUi(this);
    ui->treeViewFiles->setModel(ImportedFileReportManager::instance());
    ui->treeViewFiles->header()->resizeSection(0, 300);
    _connectSlots();
    ui->treeViewFiles->expandAll();
}
//----------------------------------------------------------
PaneImportedReportFiles::~PaneImportedReportFiles()
{
    delete ui;
}
//----------------------------------------------------------
void PaneImportedReportFiles::_connectSlots()
{
    connect(ui->buttonSort,
            &QPushButton::clicked,
            this,
            &PaneImportedReportFiles::sortAll);
    connect(ui->buttonRemove,
            &QPushButton::clicked,
            this,
            &PaneImportedReportFiles::removeSelection);
    connect(ui->buttonExpandAll,
            &QPushButton::clicked,
            this,
            &PaneImportedReportFiles::expandAll);
}
//----------------------------------------------------------
void PaneImportedReportFiles::removeSelection()
{
    auto selecedRows = ui->treeViewFiles->selectionModel()->selectedRows();
    if (selecedRows.size() > 0) {
        while (selecedRows.size() > 0) {
            auto row = selecedRows.takeLast();
            QString fileName = row.data().toString();
            QString report = row.parent().data().toString();
            QString importer = row.parent().parent().data().toString();
            QString yearDir = row.parent().parent().parent().data().toString();
            qInfo() << "Removing" << yearDir << importer << report << fileName;
            ImportedFileReportManager::instance()->removeFile(
                        fileName, yearDir, importer, report);
            qInfo() << "Removing done";
        }
    }
}
//----------------------------------------------------------
void PaneImportedReportFiles::expandAll()
{
    ui->treeViewFiles->expandAll();
}
//----------------------------------------------------------
void PaneImportedReportFiles::sortAll()
{
    ImportedFileReportManager::instance()->reorder();
}
//----------------------------------------------------------
