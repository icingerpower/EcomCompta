#include <qmessagebox.h>
#include <qfiledialog.h>
#include <QtCore/qsettings.h>
#include <QtCore/qstring.h>

#include "model/CustomerManager.h"
#include "model/bookkeeping/entries/parsers/ManagerEntryTables.h"
#include "model/bookkeeping/entries/parsers/EntryParserBankTable.h"
#include "DialogAddBankAccount.h"
#include "ui_DialogAddBankAccount.h"

//----------------------------------------------------------
DialogAddBankAccount::DialogAddBankAccount(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogAddBankAccount)
{
    ui->setupUi(this);
    ui->comboBoxBank->addItem("");
    m_bankTable = nullptr;
    m_wasAccepted = false;
    auto bankNames = ManagerEntryTables::instance()
            ->entryDisplayBanks().keys();
    ui->comboBoxBank->addItems(bankNames);
    ui->buttonBrowse->setEnabled(false);
    ui->buttonBrowseDisplay->setEnabled(false);
    connect(ui->comboBoxBank,
            &QComboBox::currentTextChanged,
            this,
            &DialogAddBankAccount::setBank);
    connect(ui->buttonBrowse,
            &QPushButton::clicked,
            this,
            &DialogAddBankAccount::browseFilePath);
    connect(ui->buttonBrowseDisplay,
            &QPushButton::clicked,
            this,
            &DialogAddBankAccount::browseFilePathDisplay);
    connect(ui->buttonLoad,
            &QPushButton::clicked,
            this,
            &DialogAddBankAccount::load);
}
//----------------------------------------------------------
DialogAddBankAccount::~DialogAddBankAccount()
{
    if (m_bankTable != nullptr) {
        m_bankTable->deleteLater();
    }
    m_bankTable = nullptr;
    delete ui;
}
//----------------------------------------------------------
QString DialogAddBankAccount::getBankFilePath() const
{
    return ui->lineEditFilePath->text();
}
//----------------------------------------------------------
QString DialogAddBankAccount::getBankFilePathDisplay() const
{
    return ui->lineEditFilePathDisplay->text();
}
//----------------------------------------------------------
QString DialogAddBankAccount::getBank() const
{
    return ui->comboBoxBank->currentText();
}
//----------------------------------------------------------
QDate DialogAddBankAccount::getDate() const
{
    return ui->dateEdit->date();
}
//----------------------------------------------------------
void DialogAddBankAccount::clear()
{
    m_bankTable->deleteLater();
    m_bankTable = nullptr;
    ui->comboBoxBank->setCurrentIndex(0);
    ui->dateEdit->setDate(QDate(2000, 1, 1));
    ui->lineEditFilePath->clear();
    ui->lineEditFilePathDisplay->clear();
}
//----------------------------------------------------------
void DialogAddBankAccount::accept()
{
    if (ui->lineEditFilePath->text().isEmpty()) {
        QMessageBox::warning(this,
                             tr("Erreur"),
                             tr("Vous devez sélectionner un fichier CSV."));
    } else if (ui->dateEdit->date() == QDate(2000, 1, 1)) {
        QMessageBox::warning(this,
                             tr("Date"),
                             tr("Il faut saisir une date."));
    } else if (ui->lineEditFilePathDisplay->text().isEmpty()) {
        QMessageBox::warning(this,
                             tr("Erreur"),
                             tr("Vous devez sélectionner un fichier PDF."));
    } else if (ui->comboBoxBank->currentText().isEmpty()) {
        QMessageBox::warning(this,
                             tr("Erreur"),
                             tr("Vous devez sélectionner une banque."));
    } else {
        if (m_bankTable != nullptr) {
            m_bankTable->deleteLater(); // TODO try without deleting any bank table because it is not allocated
        }
        m_bankTable = nullptr;
        m_wasAccepted = true;
        QDialog::accept();
    }
}
//----------------------------------------------------------
void DialogAddBankAccount::browseFilePath()
{
    QSettings settings;
    QString settingKey = "DialogAddBankAccount__browseFilePath_"
            + CustomerManager::instance()->getSelectedCustomerId();
    QString lastDirPath = settings.value(
                settingKey, QDir().absolutePath()).toString();
    QString filter = m_bankTable->fileFiltersDialog();
    QString filePath = QFileDialog::getOpenFileName(
                this, tr("Choisir un fichier"),
                lastDirPath, filter);
    if (!filePath.isEmpty()) {
        settings.setValue(settingKey, QFileInfo(filePath).path());
        ui->lineEditFilePath->setText(filePath);
        QString possiblePdfFile = filePath;
        possiblePdfFile.replace(".csv", ".pdf");
        if (QFile::exists(possiblePdfFile)) {
            ui->lineEditFilePathDisplay->setText(possiblePdfFile);
        }
        QStringList elements = QFileInfo(filePath).baseName().split("-");
        QString year;
        QString month;
        for (auto element : elements) {
            bool isNum = false;
            if (element.size() == 4 && element.toInt(&isNum)) {
                year = element;
            } else if (element.size() == 2 && element.toInt(&isNum)) {
                month = element;
            }
        }
        if (!year.isEmpty() && !month.isEmpty()) {
            QDate date(year.toInt(), month.toInt(), 1);
            ui->dateEdit->setDate(date);
        } else {
            QDate date = m_bankTable->guessDate(filePath);
            ui->dateEdit->setDate(date);
        }
    }
}
//----------------------------------------------------------
void DialogAddBankAccount::browseFilePathDisplay()
{
    QSettings settings;
    QString settingKey = "DialogAddBankAccount__browseFilePath_"
            + CustomerManager::instance()->getSelectedCustomerId();
    QString lastDirPath = settings.value(
                settingKey, QDir().absolutePath()).toString();
    QString filter = m_bankTable->fileFiltersDialog();
    QString filePath = QFileDialog::getOpenFileName(
                this, tr("Choisir un fichier"),
                lastDirPath, "PDF (*.pdf)");
    if (!filePath.isEmpty()) {
        settings.setValue(settingKey, QFileInfo(filePath).path());
        ui->lineEditFilePathDisplay->setText(filePath);
    }
}
//----------------------------------------------------------
void DialogAddBankAccount::setBank(const QString &bankName)
{
    if (bankName.isEmpty()) {
        ui->buttonBrowse->setEnabled(false);
        ui->buttonBrowseDisplay->setEnabled(false);
        m_bankTable->deleteLater();
        m_bankTable = nullptr;
    } else {
        ui->buttonBrowse->setEnabled(true);
        ui->buttonBrowseDisplay->setEnabled(true);
        auto tableDisplay = ManagerEntryTables::instance()
            ->entryDisplayBanks()[bankName];
        m_bankTable = static_cast<EntryParserBankTable *>(
                    tableDisplay)->copy();
        ui->tableView->setModel(m_bankTable);
    }
}
//----------------------------------------------------------
void DialogAddBankAccount::load()
{
    QString filePath = ui->lineEditFilePath->text();
    QDate date = ui->dateEdit->date();
    if (date.year() == 2000) {
        QMessageBox::warning(this,
                             tr("Date"),
                             tr("Il faut saisir une date."));
    } else if (filePath.isEmpty()) {
        QMessageBox::warning(this,
                             tr("Erreur"),
                             tr("Vous devez sélectionner un fichier."));
    } else {
        m_bankTable->addFilePath(
                    date.year(),
                    date.month(),
                    ui->lineEditFilePath->text(),
                    ui->lineEditFilePathDisplay->text(),
                    false);
    }
}
//----------------------------------------------------------
bool DialogAddBankAccount::wasAccepted() const
{
    return m_wasAccepted;
}
//----------------------------------------------------------
