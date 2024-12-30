#include <qmessagebox.h>

#include "DialogAddAccountAmazon.h"
#include "ui_DialogAddAccountAmazon.h"

//----------------------------------------------------------
DialogAddAccountAmazon::DialogAddAccountAmazon(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogAddAccountAmazon)
{
    ui->setupUi(this);
}
//----------------------------------------------------------
DialogAddAccountAmazon::~DialogAddAccountAmazon()
{
    delete ui;
}
//----------------------------------------------------------
void DialogAddAccountAmazon::clear()
{
    ui->lineEditAmazon->clear();
    ui->lineEditFAmazon->clear();
    ui->lineEditReserve->clear();
    ui->lineEditSalesUnknown->clear();
    ui->lineEditCustomerAccount->clear();
    ui->lineEditCharge->clear();
}
//----------------------------------------------------------
QString DialogAddAccountAmazon::getAmazon() const
{
    return ui->lineEditAmazon->text();
}
//----------------------------------------------------------
QString DialogAddAccountAmazon::getFAMAZON() const
{
    return ui->lineEditFAmazon->text();
}
//----------------------------------------------------------
QString DialogAddAccountAmazon::getAccountCustomer() const
{
    return ui->lineEditCustomerAccount->text();
}
//----------------------------------------------------------
QString DialogAddAccountAmazon::getAccountReserve() const
{
    return ui->lineEditReserve->text();
}
//----------------------------------------------------------
QString DialogAddAccountAmazon::getAccountUnknownSales() const
{
    return ui->lineEditSalesUnknown->text();
}
//----------------------------------------------------------
QString DialogAddAccountAmazon::getAccountCharge() const
{
    return ui->lineEditCharge->text();
}
//----------------------------------------------------------
void DialogAddAccountAmazon::accept()
{
    if (ui->lineEditAmazon->text().isEmpty()) {
        QMessageBox::warning(this,
                             tr("Erreur"),
                             tr("Il faut saisir un amazon"));
    } else if (!ui->lineEditAmazon->text().startsWith("amazon.")) {
        QMessageBox::warning(this,
                             tr("Erreur"),
                             tr("La première valeur doit commencé par amazon."));
    } else if (ui->lineEditReserve->text().isEmpty()) {
        QMessageBox::warning(this,
                             tr("Erreur"),
                             tr("Il faut saisir une valeure non nulle pour le compte client."));
    } else if (ui->lineEditSalesUnknown->text().isEmpty()) {
        QMessageBox::warning(this,
                             tr("Erreur"),
                             tr("Il faut saisir une valeure non nulle pour le compte de la réserve amazon."));
    } else if (ui->lineEditCustomerAccount->text().isEmpty()) {
        QMessageBox::warning(this,
                             tr("Erreur"),
                             tr("Il faut saisir une valeur non nulle pour le compte des ventes non identifiées."));
    } else { //TODO check that account doesn't exist
        QDialog::accept();
    }
}
//----------------------------------------------------------

