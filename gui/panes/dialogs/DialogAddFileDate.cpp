#include <qmessagebox.h>
#include <qfiledialog.h>
#include <QtCore/qsettings.h>
#include <QtCore/qdir.h>

#include "DialogAddFileDate.h"
#include "ui_DialogAddFileDate.h"
#include "model/CustomerManager.h"

//----------------------------------------------------------
DialogAddFileDate::DialogAddFileDate(
        const QString &idSettings,
        bool yearOnly,
        bool multipleFiles,
        QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogAddFileDate)
{
    ui->setupUi(this);
    m_yearOnly = yearOnly;
    m_multipleFilesOk = multipleFiles;
    m_idSettings = idSettings;
    if (yearOnly) {
        ui->dateEdit->setDisplayFormat("yyyy");
    }
    connect(ui->buttonBrowse,
            &QPushButton::clicked,
            this,
            &DialogAddFileDate::browse);
}
//----------------------------------------------------------
DialogAddFileDate::~DialogAddFileDate()
{
    delete ui;
}
//----------------------------------------------------------
QList<int> DialogAddFileDate::getYears() const
{
    QList<int> years;
    if (!m_multipleFilesOk
            || ui->tableWidgetFileDate->isHidden()) {
        years << ui->dateEdit->date().year();
    } else {
        for (int i=0; i< ui->tableWidgetFileDate->rowCount(); ++i) {
            QDate date = ui->tableWidgetFileDate->item(
                        i, 0)->data(Qt::DisplayRole).toDate();
            years << date.year();
        }
    }
    return years;
}
//----------------------------------------------------------
QList<QDate> DialogAddFileDate::getDates() const
{
    QList<QDate> dates;
    if (!m_multipleFilesOk
            || ui->tableWidgetFileDate->isHidden()) {
        dates << ui->dateEdit->date();
    } else {
        for (int i=0; i< ui->tableWidgetFileDate->rowCount(); ++i) {
            QDate date = ui->tableWidgetFileDate->item(
                        i, 0)->data(Qt::DisplayRole).toDate();
            dates << date;
        }
    }
    return dates;
}
//----------------------------------------------------------
QStringList DialogAddFileDate::getFilePaths() const
{
    QStringList filePaths;
    if (!m_multipleFilesOk
            || ui->tableWidgetFileDate->isHidden()) {
        filePaths << ui->lineEditFilePath->text();
    } else {
        for (int i=0; i< ui->tableWidgetFileDate->rowCount(); ++i) {
            QString filePath = ui->tableWidgetFileDate->item(
                        i, 1)->data(Qt::DisplayRole).toString();
            filePaths << filePath;
        }
    }
    return filePaths;
}
//----------------------------------------------------------
void DialogAddFileDate::accept()
{
    if (!m_multipleFilesOk
            || ui->tableWidgetFileDate->isHidden()) {

        if (ui->lineEditFilePath->text().isEmpty()) {
            QMessageBox::warning(this,
                                 tr("Erreur"),
                                 tr("Il faut choisir un fichier"));
        } else if (ui->dateEdit->date() == QDate(2000, 1, 1)) {
            QString message = tr("Il faut saisir une date.");
            if (m_yearOnly) {
                message =  tr("Il faut saisir une année.");
            }
            QMessageBox::warning(
                        this, tr("Date"), message);
        } else {
            QDialog::accept();
        }
    } else {
        for (int i=0; i< ui->tableWidgetFileDate->rowCount(); ++i) {
            QDate date = ui->tableWidgetFileDate->item(
                        i, 0)->data(Qt::DisplayRole).toDate();
            if (date == QDate(2000, 1, 1)) {
                QString message = tr("Il faut saisir une date.");
                QMessageBox::warning(
                            this, tr("Date"), tr("Il faut saisir une date pour chaque ligne."));
                return;
            }
        }
        QDialog::accept();
    }
}
//----------------------------------------------------------
void DialogAddFileDate::browse()
{
    QSettings settings;
    QString key = "DialogAddFileDate-" + m_idSettings + CustomerManager::instance()->getSelectedCustomerId();
    QString lastDirPath = settings.value(
                key, QDir().absolutePath()).toString();
    QStringList filePaths;
    if (m_multipleFilesOk) {
        filePaths = QFileDialog::getOpenFileNames(
                    this, tr("Choisir un ou plusieurs fichiers"),
                    lastDirPath,
                    "CSV (*.csv)");
    } else {
        QString filePath = QFileDialog::getOpenFileName(
                    this, tr("Choisir un fichier"),
                    lastDirPath,
                    "CSV (*.csv)");
        filePaths << filePath;
    }

    if (!filePaths.isEmpty()) {
        QString filePathFirst = filePaths.first();
        QFileInfo fileInfo(filePathFirst);
        settings.setValue(key, fileInfo.absoluteDir().absolutePath());
        if (filePaths.size() > 1) {
            ui->tableWidgetFileDate->setRowCount(filePaths.size());
            ui->tableWidgetFileDate->setColumnCount(2);
            ui->widgetOneFile->setHidden(true);
            ui->tableWidgetFileDate->setHidden(false);
            int i = 0;
            for (auto filePath : filePaths) {
                QFileInfo fileInfo(filePath);
                QDate date = _guessDate(fileInfo.fileName());
                if (!date.isValid()) {
                    date = QDate(2000, 1, 1);
                }
                auto itemDate = new QTableWidgetItem();
                itemDate->setData(Qt::DisplayRole, date);
                ui->tableWidgetFileDate->setItem(
                            i, 0, itemDate);
                ui->tableWidgetFileDate->setItem(
                            i, 1,
                            new QTableWidgetItem(filePath));
                ++i;
            }
        } else {
            ui->tableWidgetFileDate->setHidden(true);
            ui->widgetOneFile->setHidden(false);
            QDate date = _guessDate(fileInfo.fileName());
            if (date.isValid()) {
                ui->dateEdit->setDate(date);
            }
            ui->lineEditFilePath->setText(filePathFirst);
        }
    }
}
//----------------------------------------------------------
QDate DialogAddFileDate::_guessDate(const QString &fileName) const
{
    if (fileName.contains("__")) {
        QStringList elements = fileName.split("__");
        QDate date = QDate::fromString(elements[0], "yyyy-MM-dd");
        return date;
    } else if (fileName.count("-") > 1) {
        QStringList elements = fileName.split("-");
        bool isNum = false;
        int year = elements[0].toInt(&isNum);
        if (isNum) {
            int month = elements[1].toInt(&isNum);
            if (isNum) {
                int day = elements[2].toInt(&isNum);
                if (!isNum) {
                    day = 1;
                }
                return QDate(year, month, day);
            } else {
                return QDate();
            }
        } else {
            return QDate();
        }
    }
    return QDate();
}
//----------------------------------------------------------

