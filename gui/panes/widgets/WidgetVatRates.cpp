#include "WidgetVatRates.h"
#include "ui_WidgetVatRates.h"

#include "model/vat/VatRatesModel.h"
#include "model/vat/VatRatesModelDates.h"
#include "../common/gui/delegates/DelegateCountries.h"

//----------------------------------------------------------
WidgetVatRates::WidgetVatRates(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WidgetVatRates)
{
    ui->setupUi(this);
    m_modelVatDates = nullptr;
    auto delegate = new DelegateCountries(
                [](const QModelIndex &index) -> bool {
        return index.column() == 2;
    } , this);
    delegate->setUeOnly(true);
    ui->tableDateRates->setItemDelegate(delegate);
    connect(ui->buttonAdd,
            &QPushButton::clicked,
            this,
            &WidgetVatRates::add);
    connect(ui->buttonRemove,
            &QPushButton::clicked,
            this,
            &WidgetVatRates::remove);
}
//----------------------------------------------------------
WidgetVatRates::~WidgetVatRates()
{
    delete ui;
}
//----------------------------------------------------------
void WidgetVatRates::setModels(
        VatRatesModel *ratesModel,
        VatRatesModelDates *ratesModelDate)
{
    m_modelVatDates = ratesModelDate;
    ui->tableVatRates->setModel(ratesModel);
    ui->tableDateRates->setModel(ratesModelDate);
}
//----------------------------------------------------------
void WidgetVatRates::add()
{
    m_modelVatDates->add();
}
//----------------------------------------------------------
void WidgetVatRates::remove()
{
    auto selIndexes = ui->tableDateRates->selectionModel()->selectedIndexes();
    QList<int> rows;
    for (auto sel : selIndexes) {
        rows << sel.row();
    }
    rows = rows.toSet().toList();
    std::sort(rows.begin(), rows.end());
    while (rows.size() > 0) {
        auto lastRow = rows.takeLast();
        m_modelVatDates->deleteRow(lastRow);
    }
}
//----------------------------------------------------------

