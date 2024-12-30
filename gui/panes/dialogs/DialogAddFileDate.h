#ifndef DIALOGADDFILEDATE_H
#define DIALOGADDFILEDATE_H

#include <QDialog>

namespace Ui {
class DialogAddFileDate;
}

class DialogAddFileDate : public QDialog
{
    Q_OBJECT

public:
    explicit DialogAddFileDate(
            const QString &idSettings,
            bool yearOnly = false,
            bool multipleFiles = true,
            QWidget *parent = nullptr);
    ~DialogAddFileDate() override;
    QList<int> getYears() const;
    QList<QDate> getDates() const;
    QStringList getFilePaths() const;

public slots:
    void accept() override;
    void browse();

private:
    Ui::DialogAddFileDate *ui;
    bool m_yearOnly;
    bool m_multipleFilesOk;
    QString m_idSettings;
    QDate _guessDate(const QString &fileName) const;
};

#endif // DIALOGADDFILEDATE_H
