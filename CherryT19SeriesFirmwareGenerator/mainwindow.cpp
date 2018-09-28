#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "defaultBootloaderCode.h"
#include <QLayout>
#include <QFile>
#include <QFileDialog>
#include <QTextStream>
#include <QDateTime>
#include <QDir>
#include <QLabel>
#include <QProcess>
#include <QMessageBox>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->mainToolBar->close();

    componentsInitialization();


    //global layout
    auto layoutGlobal = new QVBoxLayout;
    layoutGlobal->addWidget(m_cbUseDefaultBootloader);
    layoutGlobal->addWidget(m_leBootloaderInfo);
    layoutGlobal->addWidget(m_btnLoadBootloader);
    layoutGlobal->addWidget(m_leFileInfo);
    layoutGlobal->addWidget(m_btnChooseFile);
    layoutGlobal->addWidget(m_btnGenerate);
    ui->centralWidget->setLayout(layoutGlobal);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::componentsInitialization(void)
{
    setWindowTitle(tr("CherryT19SeriesFirmwareGenerator"));

    auto labelAuthorInfo = new QLabel;
    labelAuthorInfo->setStatusTip(tr("click to view source code on my github"));
    labelAuthorInfo->setOpenExternalLinks(true);
    labelAuthorInfo->setText(QString::fromLocal8Bit("<style> a {text-decoration: none} </style> <a href = https://www.github.com/bingshuizhilian/QTPROJECTS> contact author </a>"));
    labelAuthorInfo->show();
    ui->statusBar->addPermanentWidget(labelAuthorInfo);

    m_btnChooseFile = new QPushButton(tr("load .S19 file"));
    connect(m_btnChooseFile, &m_btnChooseFile->clicked, this, &selectFile);
    m_leFileInfo = new QLineEdit;
    m_leFileInfo->setReadOnly(true);

    m_btnLoadBootloader = new QPushButton(tr("load bootloader"));
    connect(m_btnLoadBootloader, &m_btnLoadBootloader->clicked, this, &loadBootloader);
    m_leBootloaderInfo = new QLineEdit;
    m_leBootloaderInfo->setReadOnly(true);

    m_cbUseDefaultBootloader = new QCheckBox(tr("use default boot code"));
    connect(m_cbUseDefaultBootloader, &m_cbUseDefaultBootloader->stateChanged, this, &useDefaultBootloader);
    m_cbUseDefaultBootloader->setStatusTip(tr("use default boot code when checked"));
    m_cbUseDefaultBootloader->setChecked(true);

    m_btnGenerate = new QPushButton(tr("generate"));
    connect(m_btnGenerate, &m_btnGenerate->clicked, this, &generateFirmwareWithBootloader);
}

void MainWindow::useDefaultBootloader()
{
    if(m_cbUseDefaultBootloader->isChecked())
    {
        m_btnLoadBootloader->setEnabled(false);
        m_leBootloaderInfo->setText(tr("default Cherry T19 series boot code is loaded"));
    }
    else
    {
        m_btnLoadBootloader->setEnabled(true);
        m_leBootloaderInfo->clear();
    }
}

void MainWindow::loadBootloader()
{
    QString fileName = QFileDialog::getOpenFileName();
    m_leBootloaderInfo->setText(fileName);

    qDebug()<<fileName<<endl;
}

void MainWindow::selectFile()
{
    //定义文件对话框类
    QFileDialog* fileDialog = new QFileDialog(this);
    //定义文件对话框标题
    fileDialog->setWindowTitle(tr("choose .S19 file"));
    //设置默认文件路径
    fileDialog->setDirectory(".");
    //设置文件过滤器
    fileDialog->setNameFilter(tr("*.S19"));
    //设置可以选择多个文件,默认为只能选择一个文件QFileDialog::ExistingFile
    fileDialog->setFileMode(QFileDialog::ExistingFile);
    //设置视图模式
    fileDialog->setViewMode(QFileDialog::Detail);
    //打印所有选择的文件的路径
    QStringList fileNames;

    if(fileDialog->exec())
    {
        fileNames = fileDialog->selectedFiles();

        fileInfo.clear();
        fileInfo.push_back(fileNames.first());
        fileInfo.push_back(QFileInfo(fileNames.first()).absolutePath());
        fileInfo.push_back(QFileInfo(fileNames.first()).fileName());

        m_leFileInfo->setText(fileNames.first());
    }

    for(auto tmp:fileInfo)
        qDebug()<<tmp<<endl;
}

void MainWindow::generateFirmwareWithBootloader()
{
    if(m_leBootloaderInfo->text().isEmpty())
    {
        QMessageBox::warning(this, "Warnning", "Please select a bootloader file", QMessageBox::Yes);
        return;
    }

    if(m_leFileInfo->text().isEmpty())
    {
        QMessageBox::warning(this, "Warnning", "Please select a .S19 file", QMessageBox::Yes);
        return;
    }

    //获取bootloader文件
    QString bootloaderCodeString;
    if(!m_cbUseDefaultBootloader->isChecked())
    {
        QFile bootloaderFile(m_leBootloaderInfo->text());
        if(!bootloaderFile.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            QMessageBox::warning(this, "Warnning", "Cannot open " + m_leBootloaderInfo->text(), QMessageBox::Yes);
            return;
        }

        QTextStream bootloaderFileIn(&bootloaderFile);
        QStringList bootloaderCodeStringList;
        while(!bootloaderFileIn.atEnd())
        {
            QString readStr = bootloaderFileIn.readLine();

            if(!readStr.isEmpty())
                bootloaderCodeStringList.push_back(readStr);
        }

        bootloaderCodeString = bootloaderCodeStringList.join('\n');
        bootloaderCodeString += '\n';
        bootloaderFile.close();
    }
    else
    {
        bootloaderCodeString = DEFAULT_BOOTLOADER_CODE;
    }

    qDebug()<<bootloaderCodeString<<endl;

    //获取.S19原文件
    QString fileName =fileInfo.at(ABSOLUTE_FILE_PATH);
    QFile s19File(fileName);
    if(!s19File.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::warning(this, "Warnning", "Cannot open " + fileName, QMessageBox::Yes);
        return;
    }
    QTextStream s19FileIn(&s19File);
    QStringList orignalS19FileStringList;
    while(!s19FileIn.atEnd())
    {
        QString readStr = s19FileIn.readLine();

        if(!readStr.isEmpty())
            orignalS19FileStringList.push_back(readStr);
    }
    s19File.close();

    for(auto& elem:orignalS19FileStringList)
        elem += '\n';

    //将bootloader合成进原始S19文件
    int targetIndex = orignalS19FileStringList.indexOf(TARGET_STRING_AFTER_GENERATING);
    if(-1 != targetIndex)
    {
        QMessageBox::warning(this, "Warnning", "please check the .S19 file, maybe bootloader is already exist", QMessageBox::Yes);
        return;
    }

    int replaceIndex = orignalS19FileStringList.indexOf(REPLACE_STRING);
    if(-1 == replaceIndex)
    {
        QMessageBox::warning(this, "Warnning", "please check the .S19 file, interrupt vector table line doesn't exist", QMessageBox::Yes);
        return;
    }
    orignalS19FileStringList.replace(replaceIndex, bootloaderCodeString);

    //存储生成的含bootloader的文件
    QString tmpFileName = fileInfo.at(FILE_NAME);
    QString timeInfo = QDateTime::currentDateTime().toString("yyyy-MM-dd_HH-mm-ss");
    tmpFileName = tmpFileName.left(tmpFileName.size() - 4);
    tmpFileName += "_withBootloader_" + timeInfo + ".S19";

    qDebug()<<tmpFileName<<endl;

    QString folderName = "/generatedFirmwaresWithBootloader/";
    QString dirPath = fileInfo.at(ABSOLUTE_PATH) + folderName;
    QDir dir(dirPath);
    if(!dir.exists())
        dir.mkdir(dirPath);

    qDebug()<<dirPath<<endl;

    QString newFilePathName = dirPath + tmpFileName;
    QFile newFile(newFilePathName);
    if(!newFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QMessageBox::warning(this, "Warnning", "Cannot open " + newFilePathName, QMessageBox::Yes);
        return;
    }

    QTextStream out(&newFile);
    for(auto elem:orignalS19FileStringList)
    {
        if(elem != "\n")
            out << elem;
    }
    newFile.close();

    qDebug()<<newFilePathName<<endl;

    //WINDOWS环境下，选中该文件
#ifdef WIN32
    QProcess process;
    QString openFileName = newFilePathName;

    openFileName.replace("/", "\\");    //***这句windows下必要***
    process.startDetached("explorer /select," + openFileName);
#endif
}









