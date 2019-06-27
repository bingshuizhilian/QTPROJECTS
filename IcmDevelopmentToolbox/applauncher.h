#ifndef APPLAUNCHER_H
#define APPLAUNCHER_H

#include "firmwaregenerator.h"
#include "bitmapprocess.h"
#include "canlogseparator.h"
#include <QDialogButtonBox>

class AppLancher : public QDialogButtonBox
{
public:
    explicit AppLancher(QWidget *parent = 0);
    ~AppLancher();

private:
    QPushButton* btn_appFirmwareGenerator;
    QPushButton* btn_appBmpToCArray;
    QPushButton* btn_appCanLogSeparator;
    FirmwareGenerator* appFirmwareGenerator;
    BitmapProcess* appBmpToCArray;
    CanLogSeparator* appCanLogSeparator;
};

#endif // LAUNCHER_H
