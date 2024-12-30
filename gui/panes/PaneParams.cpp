#include "PaneParams.h"
#include "ui_PaneParams.h"

//----------------------------------------------------------
PaneParams::PaneParams(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PaneParams)
{
    ui->setupUi(this);
    ui->stackedWidget->hide();
    _connectSlots();
    #ifndef QT_DEBUG
    ui->stackedWidget->removeWidget(ui->pageFees);
    ui->listWidget->takeItem(6);
    #endif
}
//----------------------------------------------------------
PaneParams::~PaneParams()
{
    delete ui;
}
//----------------------------------------------------------
void PaneParams::_connectSlots()
{
    connect(ui->listWidget,
            &QListWidget::currentRowChanged,
            [this](int) {
        _showParams();
    });
}
//----------------------------------------------------------
void PaneParams::_showParams()
{
    ui->stackedWidget->show();
    ui->listWidget->parentWidget()->layout()->removeItem(ui->horizontalSpacer);
}
//----------------------------------------------------------
