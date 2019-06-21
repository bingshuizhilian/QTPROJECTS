#include "applauncher.h"
#include <QDebug>

AppLancher::AppLancher(QWidget *parent) :
    QDialogButtonBox(parent),
    appGenerate(new MainWindow),
    appBmpToCArray(new BitmapProcess)
{
    setWindowTitle(QString::fromLocal8Bit("Launcher"));
    btn_appGenerate = this->addButton(QString::fromLocal8Bit("固件生成"), QDialogButtonBox::AcceptRole);
    btn_appBmpToCArray = this->addButton(QString::fromLocal8Bit("位图处理"), QDialogButtonBox::AcceptRole);
    setCenterButtons(true);
    setFixedSize(200, 46);

    connect(btn_appGenerate, &QPushButton::clicked, this, &appGenerateClicked);
    connect(btn_appBmpToCArray, &QPushButton::clicked, this, &appBmpToCArrayClicked);

    appGenerate->setWindowModality(Qt::ApplicationModal);
    appBmpToCArray->setWindowModality(Qt::ApplicationModal);
}

AppLancher::~AppLancher()
{

}

void AppLancher::appGenerateClicked()
{
    appGenerate->show();
}

void AppLancher::appBmpToCArrayClicked()
{
    appBmpToCArray->show();
}
