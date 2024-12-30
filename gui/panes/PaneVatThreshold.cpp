#include "PaneVatThreshold.h"
#include "ui_PaneVatThreshold.h"

#include "model/vat/VatThresholdModel.h"

//----------------------------------------------------------
PaneVatThreshold::PaneVatThreshold(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PaneVatThreshold)
{
    ui->setupUi(this);
    ui->tableView->setModel(VatThresholdModel::instance());
}
//----------------------------------------------------------
PaneVatThreshold::~PaneVatThreshold()
{
    delete ui;
}
//----------------------------------------------------------
