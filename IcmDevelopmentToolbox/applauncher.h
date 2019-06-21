#ifndef APPLAUNCHER_H
#define APPLAUNCHER_H

#include "mainwindow.h"
#include "bitmapprocess.h"
#include <QDialogButtonBox>

class AppLancher : public QDialogButtonBox
{
public:
    explicit AppLancher(QWidget *parent = 0);
    ~AppLancher();

private:
    QPushButton* btn_appGenerate;
    QPushButton* btn_appBmpToCArray;
    MainWindow* appGenerate;
    BitmapProcess* appBmpToCArray;

private slots:
    void appGenerateClicked();
    void appBmpToCArrayClicked();
};

#endif // LAUNCHER_H
