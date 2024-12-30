#include "PaneImportation.h"
#include "ui_PaneImportation.h"


#include "model/orderimporters/AbstractOrderImporter.h"
#include "model/orderimporters/OrderImporterCustomManager.h"
#include "widgets/OrderImporterWidget.h"
#include "widgets/OrderReportsWidget.h"

//----------------------------------------------------------
PaneImportation::PaneImportation(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PaneImportation)
{
    ui->setupUi(this);
    auto importers = AbstractOrderImporter::allImporters();
    for (auto importer : importers) {
        ui->listSaleChannels->addItem(importer->name());
        auto importerWidget = new OrderImporterWidget(ui->stackedChannellWidget);
        ui->stackedChannellWidget->addWidget(importerWidget);
        importerWidget->init(importer);
    }
    ui->listSaleChannels->setCurrentRow(0);
    //auto model = new QFileSystemModel(this);
    //model->setReadOnly(false);
    //model->setRootPath("/home/cedric/Documents");
    //ui->treeViewFolder->setModel(model);
    //ui->treeViewFolder->setRootIndex(model->index("/home/cedric/Documents"));
}
//----------------------------------------------------------
PaneImportation::~PaneImportation()
{
    delete ui;
}
//----------------------------------------------------------

