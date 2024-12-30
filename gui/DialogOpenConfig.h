#ifndef DIALOGOPENCONFIG_H
#define DIALOGOPENCONFIG_H

#include <QDialog>

namespace Ui {
class DialogOpenConfig;
}

class DialogOpenConfig : public QDialog
{
    Q_OBJECT

public:
    static QString KEY_RECENTS;
    explicit DialogOpenConfig(QWidget *parent = nullptr);
    ~DialogOpenConfig();
    QString dirPath() const;
    bool wasAccepted() const;

public slots:
    void clearRecentSelected();
    void openRecent();
    void browseAndOpen();
    void accept() override;
    void reject() override;

private:
    Ui::DialogOpenConfig *ui;
    void _connectSlots();
    QString m_dirPath;
};

#endif // DIALOGOPENCONFIG_H
