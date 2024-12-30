#include "PaneExportImport.h"
#include "ui_PaneExportImport.h"

//----------------------------------------------------------
PaneExportImport::PaneExportImport(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PaneExportImport)
{
    ui->setupUi(this);
}
//----------------------------------------------------------
PaneExportImport::~PaneExportImport()
{
    delete ui;
}
//----------------------------------------------------------
