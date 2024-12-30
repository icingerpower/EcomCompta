#ifndef PANEIMPORTATION_H
#define PANEIMPORTATION_H

#include <QWidget>

namespace Ui {
class PaneImportation;
}

class PaneImportation : public QWidget
{
    Q_OBJECT

public:
    explicit PaneImportation(QWidget *parent = nullptr);
    ~PaneImportation();

private:
    Ui::PaneImportation *ui;
};

#endif // PANEIMPORTATION_H
