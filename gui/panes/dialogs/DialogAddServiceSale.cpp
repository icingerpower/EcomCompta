#include <QMessageBox>

#include "model/SettingManager.h"
#include "model/CustomerManager.h"

#include "DialogAddServiceSale.h"
#include "ui_DialogAddServiceSale.h"

//----------------------------------------------------------
DialogAddServiceSale::DialogAddServiceSale(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogAddServiceSale)
{
    ui->setupUi(this);
    m_accepted = false;
    ui->comboBoxCurrency->addItems(
                SettingManager::instance()->currencies());
    ui->comboBoxCurrency->setCurrentText(
                CustomerManager::instance()->getSelectedCustomerCurrency());
    m_origDate = ui->dateEdit->date();
}
//----------------------------------------------------------
DialogAddServiceSale::~DialogAddServiceSale()
{
    delete ui;
}
//----------------------------------------------------------
void DialogAddServiceSale::init(
        const QDate &date,
        const QString &label,
        double amount,
        const QString &currency)
{
    ui->dateEdit->setDate(date);
    ui->lineEditReference->setText(label);
    ui->spinBoxAmountUnit->setValue(amount);
    ui->spinBoxUnits->setValue(1);
    ui->lineEditTitle->setText("Commission sur publicité");
    ui->comboBoxCurrency->setCurrentText(currency);
}
//----------------------------------------------------------
bool DialogAddServiceSale::wasAccepted()
{
    return m_accepted;
}
//----------------------------------------------------------
QDate DialogAddServiceSale::getDate() const
{
    return ui->dateEdit->date();
}
//----------------------------------------------------------
QString DialogAddServiceSale::getReference() const
{
    return ui->lineEditReference->text();
}
//----------------------------------------------------------
QString DialogAddServiceSale::getCurrency() const
{
    return ui->comboBoxCurrency->currentText();
}
//----------------------------------------------------------
double DialogAddServiceSale::getAmountUnit() const
{
    return ui->spinBoxAmountUnit->value();
}
//----------------------------------------------------------
double DialogAddServiceSale::getUnits() const
{
    return ui->spinBoxUnits->value();
}
//----------------------------------------------------------
QString DialogAddServiceSale::getTitle() const
{
    return ui->lineEditTitle->text();
}
//----------------------------------------------------------
void DialogAddServiceSale::accept()
{
    if(ui->lineEditReference->text().isEmpty()) {
        QMessageBox::information(
                    this,
                    QObject::tr("Référence manquante"),
                    QObject::tr("Il faut saisir la référence de la vente"));
    } else if(ui->lineEditTitle->text().isEmpty()) {
        QMessageBox::information(
                    this,
                    QObject::tr("Titre manquant"),
                    QObject::tr("Il faut saisir le titre du service"));
    } else if(qAbs(ui->spinBoxAmountUnit->value()) < 0.001) {
        QMessageBox::information(
                    this,
                    QObject::tr("Montant nul"),
                    QObject::tr("Il faut saisir un montant"));
    } else if(ui->dateEdit->date() == m_origDate) {
        QMessageBox::information(
                    this,
                    QObject::tr("Date"),
                    QObject::tr("Il faut saisir une date correcte"));
    } else {
        m_accepted = true;
        QDialog::accept();
    }

}
//----------------------------------------------------------
void DialogAddServiceSale::reject()
{
    ui->lineEditReference->clear();
    m_accepted = false;
    QDialog::reject();
}
//----------------------------------------------------------
