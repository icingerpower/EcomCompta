#ifndef PANEIMPORTEDFILE_H
#define PANEIMPORTEDFILE_H

#include <QtWidgets/qwidget.h>

namespace Ui {
class PaneImportedFile;
}

class PaneImportedFile : public QWidget
{
    Q_OBJECT

public:
    explicit PaneImportedFile(QWidget *parent = nullptr);
    ~PaneImportedFile();

private:
    Ui::PaneImportedFile *ui;
};

#endif // PANEIMPORTEDFILE_H
