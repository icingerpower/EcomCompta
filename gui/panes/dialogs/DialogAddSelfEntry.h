#ifndef DIALOGADDSELFENTRY_H
#define DIALOGADDSELFENTRY_H

#include <QDialog>

namespace Ui {
class DialogAddSelfEntry;
}

class DialogAddSelfEntry : public QDialog
{
    Q_OBJECT

public:
    explicit DialogAddSelfEntry(QWidget *parent = nullptr);
    ~DialogAddSelfEntry() override;
    void clear();
    QString title() const;
    QString account() const;

public slots:
    void accept() override;

private:
    Ui::DialogAddSelfEntry *ui;
};

#endif // DIALOGADDSELFENTRY_H
