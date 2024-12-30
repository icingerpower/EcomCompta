#include <qmessagebox.h>

#include "DialogAddAccountFees.h"
#include "ui_DialogAddAccountFees.h"

//----------------------------------------------------------
DialogAddAccountFees::DialogAddAccountFees(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogAddAccountFees)
{
    ui->setupUi(this);
}
//----------------------------------------------------------
DialogAddAccountFees::~DialogAddAccountFees()
{
    delete ui;
}
//----------------------------------------------------------
QString DialogAddAccountFees::getAccountNumber() const
{
    return ui->lineEditAccountNumber->text().trimmed();
}
//----------------------------------------------------------
QString DialogAddAccountFees::getAccountLabel() const
{
    return ui->lineEditLabel->text().trimmed();
}
//----------------------------------------------------------
void DialogAddAccountFees::clear()
{
    ui->lineEditLabel->clear();
    ui->lineEditAccountNumber->clear();
}
//----------------------------------------------------------
void DialogAddAccountFees::accept()
{
    if (ui->lineEditAccountNumber->text().trimmed().isEmpty()) {
        QMessageBox::warning(this,
                             tr("Erreur"),
                             tr("Il faut saisir le numéro de compte"));
    } else if (ui->lineEditLabel->text().trimmed().isEmpty()) {
        QMessageBox::warning(this,
                             tr("Erreur"),
                             tr("Il faut saisir le titre du compte"));
    } else {
        QDialog::accept();
    }
}
//----------------------------------------------------------
