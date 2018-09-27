#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QLayout>
#include <QFile>
#include <QFileDialog>
#include <QTextStream>
#include <QDateTime>
#include <QDir>
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

    m_btnChooseFile = new QPushButton(tr("choose file"));
    connect(m_btnChooseFile, &m_btnChooseFile->clicked, this, &selectFile);

    m_leFileInfo = new QLineEdit;
    m_leFileInfo->setReadOnly(true);

    m_btnGenerate = new QPushButton(tr("generate"));
    connect(m_btnGenerate, &m_btnGenerate->clicked, this, &generateBootloader);
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

void MainWindow::generateBootloader()
{
    if(m_leFileInfo->text().isEmpty())
    {
        QMessageBox::warning(this, "Warnning", "Please select a .S19 file first", QMessageBox::Yes);
        return;
    }

    //获取原文件
    QString fileName =fileInfo.at(ABSOLUTE_FILE_PATH);
    QFile file(fileName);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::warning(this, "Warnning", "Cannot open " + fileName, QMessageBox::Yes);
        return;
    }

    QTextStream in(&file);
    QString orignalFile = in.readAll();
    file.close();

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
    out << orignalFile + "\n\nthis is a test!";
    newFile.close();

    qDebug()<<newFilePathName<<endl;
}









