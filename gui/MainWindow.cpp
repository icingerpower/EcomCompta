#include "MainWindow.h"
#include "ui_MainWindow.h"

#include "model/CustomerManager.h"

//==========================================================
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->comboBoxCustomer->setModel(
                CustomerManager::instance());
    _connectSlots();
    int nCustomers = CustomerManager::instance()->nCustomers();
    if (nCustomers == 0) {
        ui->tabCustomers->addFirstCustomerForcing();
    }
    #ifndef QT_DEBUG
    ui->tabWidget->removeTab(5);
    ui->tabWidget->removeTab(2);
    //ui->tabAmazonPayments->deleteLater();
    //ui->tabInventaire->deleteLater();
    #endif
}
//==========================================================
MainWindow::~MainWindow()
{
    delete ui;
}
//==========================================================
void MainWindow::_connectSlots()
{
    connect(ui->comboBoxCustomer,
            &QComboBox::currentTextChanged,
            CustomerManager::instance(),
            &CustomerManager::selectCustomer);
}
//==========================================================
