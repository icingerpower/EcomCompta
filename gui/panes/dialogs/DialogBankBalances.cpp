#include <QTextStream>

#include "model/bookkeeping/entries/parsers/ManagerEntryTables.h"
#include "model/bookkeeping/entries/parsers/EntryParserBankTable.h"
#include "model/bookkeeping/SettingBookKeeping.h"
#include "model/bookkeeping/TableBankBeginBalances.h"
#include "model/SettingManager.h"

#include "DialogBankBalances.h"
#include "ui_DialogBankBalances.h"

//----------------------------------------------------------
DialogBankBalances::DialogBankBalances(const QString &year, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogBankBalances)
{
    ui->setupUi(this);
    ui->tableBeginBalances->setModel(TableBankBeginBalances::instance());
    m_year = year;
    const auto &banks = ManagerEntryTables::instance()->entryDisplayBanks();
    for (const auto &bank : banks) {
        const QString &name = bank->name();
        const QString &journal = ManagerEntryTables::instance()->journalName(
                    bank->name());
        QString account = bank->bankParser()->account();
        QString displayName(name);
        displayName += " (";
        displayName += journal;
        displayName += ")";
        ui->listBanks->addItem(name);
    }
    _compute();
    connect(ui->listBanks,
            &QListWidget::currentRowChanged,
            this,
            &DialogBankBalances::onSelectedRow);
    connect(TableBankBeginBalances::instance(),
            &QAbstractItemModel::dataChanged,
            this,
            [this](){
        _compute();
        onSelectedRow(ui->listBanks->currentRow());
    });
}
//----------------------------------------------------------
DialogBankBalances::~DialogBankBalances()
{
    delete ui;
}
//----------------------------------------------------------
void DialogBankBalances::onSelectedRow(int row)
{
    const QList<QStringList> &table = m_listOfTable[row];
    ui->tableBalances->clearContents();
    ui->tableBalances->setRowCount(table.size());
    for (int i=0; i<table.size(); ++i) {
        for (int j=0; j<table[i].size(); ++j) {
            ui->tableBalances->setItem(
                        i, j, new QTableWidgetItem(table[i][j]));
        }
    }
}
//----------------------------------------------------------
void DialogBankBalances::_compute()
{
    m_listOfTable.clear();
    const auto &banks = ManagerEntryTables::instance()->entryDisplayBanks();
    QDir bookKeepingDir = SettingBookKeeping::instance()->dirPath();
    bookKeepingDir.cd(m_year);
    QStringList months{"01", "02", "03", "04", "05", "06", "07", "08", "09", "10", "11", "12"};
    for (const auto &bank : banks) {
        auto bankParser = bank->bankParser();
        const QString &name = bank->name();
        const QString &journal = ManagerEntryTables::instance()->journalName(
                    bank->name());
        QString account = bankParser->account();
        QDir bankDirAccounting = bookKeepingDir.filePath(journal);
        QDir bankDir = SettingManager::instance()->bookKeepingDirBank(
                bankParser->nameFolder(), m_year.toInt());
        QList<QStringList> table;
        double amount = TableBankBeginBalances::instance()->getBeginAmount(name);
        for (const auto &month : months) {
            QString dateMonth(m_year);
            dateMonth += "-";
            dateMonth += month;
            QString csvFileNameAccounting(journal);
            csvFileNameAccounting += "-";
            csvFileNameAccounting += dateMonth;
            csvFileNameAccounting += ".csv";
            QString csvFileNameBank = bankParser->fileName(m_year, month);
            QDir csvDirAccounting = bankDirAccounting.filePath(month);
            if (csvDirAccounting.exists()) {
                const QString &csvFilePathAccounting = csvDirAccounting.filePath(csvFileNameAccounting);
                const QString &csvFilePathBank = bankDir.filePath(csvFileNameBank);
                auto entrySets = bank->bankParser()->entrySetsFromFilePath(csvFilePathBank);
                QString currency("EUR");
                if (entrySets.size() > 0) {
                    currency = entrySets[0]->currencyOrig();
                }
                if (currency == "EUR") {
                    QFile file(csvFilePathAccounting);
                    if (file.open(QFile::ReadOnly)) {
                        QTextStream stream(&file);
                        const QStringList &lines = stream.readAll().split("\n");
                        for (const auto &line : lines) {
                            const QStringList &elements = line.split(";");
                            if (elements[2] == account) {
                                double debit = elements[3].toDouble();
                                double credit = elements[4].toDouble();
                                amount += debit;
                                amount -= credit;
                            }
                        }
                        file.close();
                    }
                } else {
                    for (const auto &entrySet : entrySets) {
                        const auto &entries = entrySet->entries();
                        for (const auto &entry : entries) {
                            if (entry.account() == account) {
                                double debit = entry.debitOrigDouble();
                                double credit = entry.creditOrigDouble();
                                amount += debit;
                                amount -= credit;
                            }
                        }
                    }
                }
                QStringList row{dateMonth, QString::number(amount, 'f', 2), currency};
                table << row;
            }
        }
        m_listOfTable << table;
    }
}
//----------------------------------------------------------
