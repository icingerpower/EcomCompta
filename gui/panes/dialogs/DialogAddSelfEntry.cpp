#include <qmessagebox.h>

#include "DialogAddSelfEntry.h"
#include "ui_DialogAddSelfEntry.h"

//----------------------------------------------------------
DialogAddSelfEntry::DialogAddSelfEntry(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogAddSelfEntry)
{
    ui->setupUi(this);
}
//----------------------------------------------------------
DialogAddSelfEntry::~DialogAddSelfEntry()
{
    delete ui;
}
//----------------------------------------------------------
void DialogAddSelfEntry::clear()
{
    ui->lineEditTitle->clear();
    ui->lineEditAccount->clear();
}
//----------------------------------------------------------
QString DialogAddSelfEntry::title() const
{
    return ui->lineEditTitle->text();
}
//----------------------------------------------------------
QString DialogAddSelfEntry::account() const
{
    return ui->lineEditAccount->text();
}
//----------------------------------------------------------
void DialogAddSelfEntry::accept()
{
    if (ui->lineEditTitle->text().isEmpty()) {
        QMessageBox::warning(this,
                             tr("Erreur"),
                             tr("Il faut saisir un titre"));
    } else if (ui->lineEditTitle->text().isEmpty()) {
        QMessageBox::warning(this,
                             tr("Erreur"),
                             tr("Il faut saisir un numéro de compte"));
    } else {
        QDialog::accept();
    }
}
//----------------------------------------------------------
