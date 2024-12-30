#include <QSettings>
#include <QFileDialog>
#include <QMessageBox>
#include <QDir>

#include "model/SettingManager.h"

#include "DialogOpenConfig.h"
#include "ui_DialogOpenConfig.h"

//----------------------------------------------------------
QString DialogOpenConfig::KEY_RECENTS = "recent-working-directory-paths";
//----------------------------------------------------------
DialogOpenConfig::DialogOpenConfig(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogOpenConfig)
{
    ui->setupUi(this);
    _connectSlots();
    QStringList paths;
    QSettings settings;
    if (settings.contains(KEY_RECENTS)) {
        paths = settings.value(KEY_RECENTS).toStringList();
    }
    ui->listRecentPaths->addItems(paths);
}
//----------------------------------------------------------
void DialogOpenConfig::_connectSlots()
{
    connect(ui->buttonBrowseAndOpen,
            &QPushButton::clicked,
            this,
            &DialogOpenConfig::browseAndOpen);
    connect(ui->buttonClearRecentSelected,
            &QPushButton::clicked,
            this,
            &DialogOpenConfig::clearRecentSelected);
    connect(ui->buttonOpenRecent,
            &QPushButton::clicked,
            this,
            &DialogOpenConfig::openRecent);
}
//----------------------------------------------------------
DialogOpenConfig::~DialogOpenConfig()
{
    delete ui;
}
//----------------------------------------------------------
QString DialogOpenConfig::dirPath() const
{
    return m_dirPath;
}

bool DialogOpenConfig::wasAccepted() const
{
    return !m_dirPath.isEmpty();
}
//----------------------------------------------------------
void DialogOpenConfig::clearRecentSelected()
{
    auto selectedItems = ui->listRecentPaths->selectedItems();
    if (selectedItems.size() == 1) {
        QSettings settings;
        int row = ui->listRecentPaths->currentRow();
        QString recent = selectedItems.first()->data(
                    Qt::DisplayRole).toString();
        QStringList paths = settings.value(KEY_RECENTS).toStringList();
        paths.removeAt(row);
        ui->listRecentPaths->takeItem(row);
        settings.setValue(KEY_RECENTS, paths);
    }
}
//----------------------------------------------------------
void DialogOpenConfig::openRecent()
{
    auto selectedItems = ui->listRecentPaths->selectedItems();
    if (selectedItems.size() == 1) {
        QString path = selectedItems[0]->text();
        if (!QDir(path).exists()) {
            QMessageBox::warning(
                        this,
                        tr("Repertoire non existante"),
                        tr("Le répertoire sélectionné n’existe pas."));
        } else {
            m_dirPath = path;
            accept();
        }
    }
}
//----------------------------------------------------------
void DialogOpenConfig::browseAndOpen()
{
    QSettings settings;
    QString settingKey = "DialogOpenConifg::browseAndOpen";
    QString lastDirPath = settings.value(
                settingKey, QDir().absolutePath()).toString();
    QString dirPath = QFileDialog::getExistingDirectory(
                this,
                tr("Chose a directory"),
                lastDirPath);
    if (!dirPath.isEmpty()) {
        settings.setValue(settingKey, QFileInfo(dirPath).path());
        m_dirPath = dirPath;
        QStringList paths = settings.value(KEY_RECENTS).toStringList();
        if (!paths.contains(dirPath)) {
            paths.insert(0, dirPath);
            settings.setValue(KEY_RECENTS, paths);
        }
        accept();
    }
}
//----------------------------------------------------------
void DialogOpenConfig::accept()
{
    QDialog::accept();
}
//----------------------------------------------------------
void DialogOpenConfig::reject()
{
    m_dirPath.clear();
    QDialog::reject();
}
//----------------------------------------------------------
