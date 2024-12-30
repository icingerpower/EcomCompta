#include "PaneFees.h"
#include "ui_PaneFees.h"

#include "model/orderimporters/AbstractOrderImporter.h"
#include "model/orderimporters/FeesAccountManager.h"
#include "model/orderimporters/FeesTableModel.h"
#include "dialogs/DialogAddAccountFees.h"

//==========================================================
PaneFees::PaneFees(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PaneFees)
{
    ui->setupUi(this);
    m_dialogAddAccount = nullptr;
    ui->tableViewAccounts->setModel(FeesAccountManager::instance());
    auto importers = AbstractOrderImporter::allImporters();
    for (auto importer : importers) {
        ui->listWidgetImporters->addItem(importer->name());
        auto tableView = new QTableView(ui->stackedWidgetImporters);
        tableView->horizontalHeader()->setStretchLastSection(true);
        auto model = FeesTableModel::instance(importer->name());
        tableView->setModel(model);
        ui->stackedWidgetImporters->addWidget(tableView);
    }
    _connectSlots();
}
//==========================================================
PaneFees::~PaneFees()
{
    delete ui;
}
//==========================================================
void PaneFees::addAccount()
{
    if (m_dialogAddAccount == nullptr) {
        m_dialogAddAccount = new DialogAddAccountFees(this);
        connect(m_dialogAddAccount,
                &DialogAddAccountFees::accepted,
                [this](){
            QString account = m_dialogAddAccount->getAccountNumber();
            QString title = m_dialogAddAccount->getAccountLabel();
            FeesAccountManager::instance()->addAccount(account, title);
        });
    } else {
        m_dialogAddAccount->clear();
    }
    m_dialogAddAccount->show();
}
//==========================================================
void PaneFees::deleteSelectedAccount()
{
    auto selection = ui->tableViewAccounts->selectionModel()->selectedRows();
    if (!selection.isEmpty()) {
        int index = selection.first().row();
        FeesAccountManager::instance()->removeAccount(index);
    }
}
//==========================================================
void PaneFees::_connectSlots()
{
    connect(ui->buttonAddAccount,
            &QPushButton::clicked,
            this,
            &PaneFees::addAccount);
    connect(ui->buttonRemoveAccount,
            &QPushButton::clicked,
            this,
            &PaneFees::deleteSelectedAccount);
    /*
    connect(ui->listWidgetImporters,
            &QListWidget::currentRowChanged,
            [this](int) {
        QString importerName = ui->listWidgetImporters->currentItem()->text();
    });
    //*/
}
//==========================================================
