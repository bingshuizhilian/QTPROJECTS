#include "applauncher.h"
#include <QDebug>

AppLancher::AppLancher(QWidget *parent) :
    QDialogButtonBox(parent),
    appFirmwareGenerator(new FirmwareGenerator),
    appBmpToCArray(new BitmapProcess),
    appCanLogSeparator(new CanLogSeparator)
{
    setWindowTitle(QString::fromLocal8Bit("Launcher"));
//    setOrientation(Qt::Vertical);
    btn_appFirmwareGenerator = addButton(QString::fromLocal8Bit("固件生成"), QDialogButtonBox::AcceptRole);
    btn_appBmpToCArray = addButton(QString::fromLocal8Bit("位图处理"), QDialogButtonBox::AcceptRole);
    btn_appCanLogSeparator = addButton(QString::fromLocal8Bit("can报文分析"), QDialogButtonBox::AcceptRole);
    setCenterButtons(true);
    setFixedSize(250, 45);
//    setFixedWidth(180);

//    btn_appGenerate->setFixedWidth(120);
//    btn_appBmpToCArray->setFixedWidth(120);

    connect(this, &clicked, this, [=](QAbstractButton* b){
        if(b == btn_appFirmwareGenerator)
            appFirmwareGenerator->show();
        else if(b == btn_appBmpToCArray)
            appBmpToCArray->show();
        else if(b == btn_appCanLogSeparator)
            appCanLogSeparator->show();

        qDebug() << width() << height();
    });

    appFirmwareGenerator->setWindowModality(Qt::ApplicationModal);
    appBmpToCArray->setWindowModality(Qt::ApplicationModal);
    appCanLogSeparator->setWindowModality(Qt::ApplicationModal);
}

AppLancher::~AppLancher()
{

}
