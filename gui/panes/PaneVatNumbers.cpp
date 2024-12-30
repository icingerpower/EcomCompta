#include "PaneVatNumbers.h"
#include "ui_PaneVatNumbers.h"
#include "model/vat/VatNumbersModel.h"
#include "model/vat/ManagerCompanyVatParams.h"
#include "gui/panes/dialogs/DialogAddVatnumber.h"
#include "../common/gui/delegates/DelegateCountries.h"

//----------------------------------------------------------
PaneVatNumbers::PaneVatNumbers(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PaneVatNumbers)
{
    ui->setupUi(this);
    ui->tableViewNumbers->setModel(VatNumbersModel::instance());
    ui->tableViewCompany->setModel(ManagerCompanyVatParams::instance());
    ui->tableViewCompany->horizontalHeader()->resizeSection(0, 250);
    ui->tableViewCompany->setItemDelegate(
                new DelegateCountries(
                    [](const QModelIndex &index) -> bool{
        return index.row() == 0 && index.column() ==1;
    }
                    , this));
    //ui->tableViewCompany->viewport()->height();
    m_dialogAddNumber = nullptr;
    if (VatNumbersModel::instance()->hasIossThreshold()) {
        ui->radioThreshold10000->setChecked(true);
    } else {
        ui->radioThreshold0->setChecked(true);
    }
    _connectSlots();
}
//----------------------------------------------------------
void PaneVatNumbers::_connectSlots()
{
    connect(ui->buttonAdd,
            &QPushButton::clicked,
            this,
            &PaneVatNumbers::addVatNumber);
    connect(ui->buttonRemove,
            &QPushButton::clicked,
            this,
            &PaneVatNumbers::removeVatNumberSelected);
    connect(VatNumbersModel::instance(),
            &VatNumbersModel::iossThresholdChanded,
            [this](bool checked) {
        ui->radioThreshold10000->setChecked(checked);
        ui->radioThreshold0->setChecked(!checked);
    });
    /*
    connect(VatNumbersModel::instance(),
            &VatNumbersModel::iossNumberChanged,
            [this](const VatNumberData &vatNumber) {
        // TODO
    });
    //*/
    connect(ui->radioThreshold0,
            &QRadioButton::toggled,
            [](bool checked) {
        VatNumbersModel::instance()->setIossThreshold(!checked);
    });
}
//----------------------------------------------------------
PaneVatNumbers::~PaneVatNumbers()
{
    delete ui;
}
//----------------------------------------------------------
void PaneVatNumbers::addVatNumber()
{
    if (m_dialogAddNumber == nullptr) {
        m_dialogAddNumber = new DialogAddVatnumber(this);
        connect(m_dialogAddNumber,
                &DialogAddVatnumber::accepted,
                [this](){
            VatNumberData vat = m_dialogAddNumber->getVatNumber();
            VatNumbersModel::instance()->add(vat);
        });
    }
    m_dialogAddNumber->clear();
    m_dialogAddNumber->show();
}
//----------------------------------------------------------
void PaneVatNumbers::removeVatNumberSelected()
{
    auto selection = ui->tableViewNumbers->selectionModel()->selectedRows();
    while (selection.size() > 0) {
        auto curSel = selection.takeLast();
        VatNumbersModel::instance()->remove(curSel);
    }
}
//----------------------------------------------------------
