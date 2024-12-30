#include <QInputDialog>
#include <QDate>

#include "PaneInvoiceSettings.h"
#include "ui_PaneInvoiceSettings.h"

#include "model/bookkeeping/invoices/SettingInvoices.h"
#include "model/bookkeeping/invoices/SettingInvoicesHeadOffice.h"

//----------------------------------------------------------
PaneInvoiceSettings::PaneInvoiceSettings(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PaneInvoiceSettings),
    UpdateToCustomer()
{
    ui->setupUi(this);
    ui->tableViewInvoicing->setModel(
                SettingInvoicesHeadOffice::instance());
    ui->tableViewParams->setModel(SettingInvoices::instance());
    _initPlainTextEdits();
    _connectSlots();
}
//----------------------------------------------------------
void PaneInvoiceSettings::_connectSlots()
{
    connect(ui->textEditBottomLaw,
            &QPlainTextEdit::textChanged,
            [this]() {
        QStringList lines
                = ui->textEditBottomLaw->toPlainText().split("\n");
        SettingInvoices::instance()->setTextBottomLaw(lines);
    });
    connect(ui->textEditBottomLegal,
            &QPlainTextEdit::textChanged,
            [this]() {
        QStringList lines
                = ui->textEditBottomLegal->toPlainText().split("\n");
        SettingInvoices::instance()->setTextBottomLegal(lines);
    });
    connect(ui->textEditCompany,
            &QPlainTextEdit::textChanged,
            [this]() {
        QStringList lines
                = ui->textEditCompany->toPlainText().split("\n");
        SettingInvoices::instance()->setAddressFrom(lines);
    });
    connect(ui->buttonAddDate,
            &QPushButton::clicked,
            this,
            &PaneInvoiceSettings::addDate);
    connect(ui->buttonRemoveDate,
            &QPushButton::clicked,
            this,
            &PaneInvoiceSettings::removeDate);
}
//----------------------------------------------------------
void PaneInvoiceSettings::_disconnectSlots()
{
    disconnect(ui->textEditBottomLaw);
    disconnect(ui->textEditBottomLegal);
    disconnect(ui->textEditCompany);
}
//----------------------------------------------------------
void PaneInvoiceSettings::_initPlainTextEdits()
{
    ui->textEditBottomLaw->setPlainText(
                SettingInvoices::instance()->textBottomLaw().join("\n"));
    ui->textEditBottomLegal->setPlainText(
                SettingInvoices::instance()->textBottomLegal().join("\n"));
    ui->textEditCompany->setPlainText(
                SettingInvoices::instance()->addressFrom().join("\n"));
}
//----------------------------------------------------------
void PaneInvoiceSettings::onCustomerSelectedChanged(
        const QString &customerId)
{
    _disconnectSlots();
    _initPlainTextEdits();
    _connectSlots();
}
//----------------------------------------------------------
QString PaneInvoiceSettings::uniqueId() const
{
    return "PaneInvoiceSettings";
}
//----------------------------------------------------------
PaneInvoiceSettings::~PaneInvoiceSettings()
{
    delete ui;
}
//----------------------------------------------------------
void PaneInvoiceSettings::addDate()
{
    SettingInvoicesHeadOffice::instance()->addDate(
                QDate::currentDate());
}
//----------------------------------------------------------
void PaneInvoiceSettings::removeDate()
{
    auto selIndexes = ui->tableViewInvoicing->selectionModel()->selectedIndexes();
    if (selIndexes.size() > 0) {
        SettingInvoicesHeadOffice::instance()
                ->removeDate(selIndexes.first());
    }
}
//----------------------------------------------------------
