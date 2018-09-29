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
#include <QInputDialog>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->mainToolBar->close();

    //初始化控件
    componentsInitialization();


    //global layout
    auto layoutGlobal = new QVBoxLayout;
    layoutGlobal->addWidget(m_cmbFunctionSwitch);
    layoutGlobal->addWidget(m_ckbUseDefaultBootloader);
    layoutGlobal->addWidget(m_leBootloaderInfo);
    layoutGlobal->addWidget(m_btnLoadBootloader);
    layoutGlobal->addWidget(m_leFileInfo);
    layoutGlobal->addWidget(m_btnChooseFile);
    layoutGlobal->addWidget(m_leDiagnosisS021);
    layoutGlobal->addWidget(m_leDiagnosisS20C);
    layoutGlobal->addWidget(m_btnGenerate);
    ui->centralWidget->setLayout(layoutGlobal);

}

MainWindow::~MainWindow()
{
    delete ui;
}

//切换功能后，控件间关联关系的处理
void MainWindow::switchFunctionPressed()
{
    switch(m_cmbFunctionSwitch->currentIndex())
    {
    case BOOTLOADER:
        m_btnGenerate->setStatusTip(tr("generate firmware with bootloader"));
        m_btnGenerate->setText(tr("generate"));
        m_leDiagnosisS021->setVisible(false);
        m_leDiagnosisS20C->setVisible(false);
        m_btnChooseFile->setVisible(true);
        m_leFileInfo->setVisible(true);
        m_ckbUseDefaultBootloader->setVisible(true);
        m_btnLoadBootloader->setVisible(true);
        m_leBootloaderInfo->setVisible(true);
        break;
    case DIAGNOSIS:
        m_btnGenerate->setStatusTip(tr("generate firmware for diagnosis"));
        m_btnGenerate->setText(tr("generate"));
        m_leDiagnosisS021->setVisible(true);
        m_leDiagnosisS20C->setVisible(true);
        m_btnChooseFile->setVisible(true);
        m_leFileInfo->setVisible(true);
        m_ckbUseDefaultBootloader->setVisible(false);
        m_btnLoadBootloader->setVisible(false);
        m_leBootloaderInfo->setVisible(false);
        break;
    case BOOTCODE2STRING:
        m_btnGenerate->setStatusTip(tr("convert boot code to string, and generate a .h file"));
        m_btnGenerate->setText(tr("load and generate"));
        m_leDiagnosisS021->setVisible(false);
        m_leDiagnosisS20C->setVisible(false);
        m_btnChooseFile->setVisible(false);
        m_leFileInfo->setVisible(false);
        m_ckbUseDefaultBootloader->setVisible(false);
        m_btnLoadBootloader->setVisible(false);
        m_leBootloaderInfo->setVisible(false);
        break;
    default:
        break;
    }
}

void MainWindow::generateButtonPressed()
{
    switch(m_cmbFunctionSwitch->currentIndex())
    {
    case BOOTLOADER:
        generateFirmwareWithBootloader();
        break;
    case DIAGNOSIS:
        generateFirmwareForDiagnosis();
        break;
    case BOOTCODE2STRING:
        generateCharArray();
        break;
    default:
        break;
    }
}

void MainWindow::useDefaultBootloaderPressed()
{
    if(m_ckbUseDefaultBootloader->isChecked())
    {
        m_btnLoadBootloader->setEnabled(false);
        m_leBootloaderInfo->setText(tr("default Cherry T19 series boot code is loaded"));
        m_leBootloaderInfo->setEnabled(false);
        m_btnLoadBootloader->setStatusTip(tr("use default boot code now"));
    }
    else
    {
        m_btnLoadBootloader->setEnabled(true);
        m_leBootloaderInfo->setEnabled(true);
        m_leBootloaderInfo->clear();
        m_btnLoadBootloader->setStatusTip(tr("select a file which contains boot code"));
    }
}

void MainWindow::loadBootloaderPressed()
{
    QString fileName = QFileDialog::getOpenFileName();
    m_leBootloaderInfo->setText(fileName);

    qDebug()<<fileName<<endl;
}

void MainWindow::selectFilePressed()
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
    if(!m_ckbUseDefaultBootloader->isChecked())
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

        bootloaderFile.close();

        bootloaderCodeString = bootloaderCodeStringList.join('\n');
        bootloaderCodeString += '\n';
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
    int targetIndex = orignalS19FileStringList.indexOf(TARGET_STRING_AFTER_GENERATING_BOOTCODE);
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

void MainWindow::generateFirmwareForDiagnosis()
{
    if(m_leFileInfo->text().isEmpty())
    {
        QMessageBox::warning(this, "Warnning", "Please select a .S19 file", QMessageBox::Yes);
        return;
    }

    if(m_leDiagnosisS021->text().isEmpty())
    {
        QMessageBox::warning(this, "Warnning", "Please input S021 data", QMessageBox::Yes);
        return;
    }
    else
    {
        if(!(m_leDiagnosisS021->text().contains("S021") || m_leDiagnosisS021->text().contains("s021")))
        {
            QMessageBox::warning(this, "Warnning", "Please check S021 data", QMessageBox::Yes);
            return;
        }
    }

/*

    if(m_leDiagnosisS20C->text().isEmpty())
    {
        QMessageBox::warning(this, "Warnning", "Please select a .S19 file", QMessageBox::Yes);
        return;
    }

    bool isOK;
    QString s20cQuery = QInputDialog::getText(NULL, "Additional data query",
                                               "Please input the CRC result code line starts with S20C:",
                                               QLineEdit::Normal,
                                               "",
                                               &isOK);
    if(isOK) {
           QMessageBox::information(NULL, "Information",
                                           "Your comment is: <b>" + s20cQuery + "</b>",
                                           QMessageBox::Yes | QMessageBox::No,
                                           QMessageBox::Yes);
    }
*/

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

    //校验原文件是否正确
    int bootValidateIndex = orignalS19FileStringList.indexOf(TARGET_STRING_AFTER_GENERATING_BOOTCODE);
    if(-1 != bootValidateIndex)
    {
        QMessageBox::warning(this, "Warnning", "please check the .S19 file, bootloader code exists", QMessageBox::Yes);
        return;
    }

    if(orignalS19FileStringList.first().contains("S021"))
    {
        QMessageBox::warning(this, "Warnning", "please check the .S19 file, S021 code is already exist in first line", QMessageBox::Yes);
        return;
    }

    int vectorValidateIndex = orignalS19FileStringList.indexOf(REPLACE_STRING);
    if(-1 == vectorValidateIndex)
    {
        QMessageBox::warning(this, "Warnning", "please check the .S19 file, interrupt vector table S10B line doesn't exist", QMessageBox::Yes);
        return;
    }

    //替换S0行，删除S10B行
    orignalS19FileStringList.replace(0, m_leDiagnosisS021->text());
    orignalS19FileStringList.removeOne(REPLACE_STRING);

    for(auto elem: orignalS19FileStringList)
        qDebug() << elem << endl;

    //flash中S224按Fxx升序排序
}

//将boot code生成为字符串常量，当boot code更新时，调用此函数将其转换为数组
void MainWindow::generateCharArray()
{
    QString filePathName = QFileDialog::getOpenFileName();
    QString filePath = QFileInfo(filePathName).absolutePath();

    qDebug()<<filePathName<<endl<<filePath<<endl;

    if(filePathName.isEmpty())
    {
        QMessageBox::warning(this, "Warnning", "generate failed, please select a file", QMessageBox::Yes);
        return;
    }

    QFile targetFile(filePathName);
    if(!targetFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::warning(this, "Warnning", "Cannot open " + filePathName, QMessageBox::Yes);
        return;
    }

    QTextStream targetFileIn(&targetFile);
    QStringList targetCodeStringList;
    while(!targetFileIn.atEnd())
    {
        QString readStr = targetFileIn.readLine();

        if(!readStr.isEmpty())
            targetCodeStringList.push_back(readStr);
    }

    targetFile.close();
    for(auto& elem:targetCodeStringList)
    {
        elem.push_front('\"');
        elem.push_back('\\');
        elem.push_back('n');
        elem.push_back('\"');
        elem.push_back('\n');
    }
    if(!targetCodeStringList.isEmpty())
        targetCodeStringList.last().insert(targetCodeStringList.last().size() - 1, ';');

    targetCodeStringList.push_front("const char* DEFAULT_BOOTLOADER_CODE =\n");
    targetCodeStringList.push_front("#define __DEFAULT_BOOTLOADER_CODE_H__\n\n");
    targetCodeStringList.push_front("#ifndef __DEFAULT_BOOTLOADER_CODE_H__\n");
    targetCodeStringList.push_back("\n#endif\n");

    for(auto& elem:targetCodeStringList)
        qDebug()<<elem<<endl;

    QString tmpFileName = "defaultBootloaderCode_";
    QString timeInfo = QDateTime::currentDateTime().toString("yyyy-MM-dd_HH-mm-ss");
    tmpFileName += timeInfo + ".h";

    QString newFilePathName = filePath + '/' + tmpFileName;
    QFile newFile(newFilePathName);
    if(!newFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QMessageBox::warning(this, "Warnning", "Cannot open " + newFilePathName, QMessageBox::Yes);
        return;
    }

    QTextStream out(&newFile);
    for(auto elem:targetCodeStringList)
        out << elem;

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

void MainWindow::componentsInitialization(void)
{
    setWindowTitle(tr("CherryT19SeriesFirmwareGenerator"));

    //窗体名称及状态栏设置
    auto labelAuthorInfo = new QLabel;
    labelAuthorInfo->setStatusTip(tr("click to view source code on my github"));
    labelAuthorInfo->setOpenExternalLinks(true);
    labelAuthorInfo->setText(QString::fromLocal8Bit("<style> a {text-decoration: none} </style> <a href = https://www.github.com/bingshuizhilian/QTPROJECTS> contact author </a>"));
    labelAuthorInfo->show();
    ui->statusBar->addPermanentWidget(labelAuthorInfo);

    //选择.S19文件按钮
    m_btnChooseFile = new QPushButton(tr("load .S19 file"));
    connect(m_btnChooseFile, &m_btnChooseFile->clicked, this, &selectFilePressed);
    m_btnChooseFile->setStatusTip(tr("select the target .S19 file without boot code"));
    //显示.S19文件名
    m_leFileInfo = new QLineEdit;
    m_leFileInfo->setReadOnly(true);

    //选择bootloader文件按钮
    m_btnLoadBootloader = new QPushButton(tr("load bootloader"));
    connect(m_btnLoadBootloader, &m_btnLoadBootloader->clicked, this, &loadBootloaderPressed);
    //显示bootloader文件名，或提示正在使用默认boot code
    m_leBootloaderInfo = new QLineEdit;
    m_leBootloaderInfo->setReadOnly(true);

    //是否使用默认boot code选项
    m_ckbUseDefaultBootloader = new QCheckBox(tr("use default boot code"));
    connect(m_ckbUseDefaultBootloader, &m_ckbUseDefaultBootloader->stateChanged, this, &useDefaultBootloaderPressed);
    m_ckbUseDefaultBootloader->setStatusTip(tr("use default boot code when checked"));
    m_ckbUseDefaultBootloader->setChecked(true);

    //生成按钮
    m_btnGenerate = new QPushButton(tr("generate"));
    connect(m_btnGenerate, &m_btnGenerate->clicked, this, &generateButtonPressed);

    //合成诊断仪firmware时需要的额外信息
    m_leDiagnosisS021 = new QLineEdit;
    m_leDiagnosisS20C = new QLineEdit;
    m_leDiagnosisS20C->setReadOnly(true);

    //功能选择下拉框
    m_cmbFunctionSwitch = new QComboBox;
    connect(m_cmbFunctionSwitch,  static_cast<void (QComboBox::*)(const QString&)>(&QComboBox::currentTextChanged), this, &switchFunctionPressed);
    m_cmbFunctionSwitch->addItem(FUNCTION_STRING_LIST.at(BOOTLOADER), FUNCTION_STRING_LIST.at(BOOTLOADER));
    m_cmbFunctionSwitch->addItem(FUNCTION_STRING_LIST.at(DIAGNOSIS), FUNCTION_STRING_LIST.at(DIAGNOSIS));
    m_cmbFunctionSwitch->addItem(FUNCTION_STRING_LIST.at(BOOTCODE2STRING), FUNCTION_STRING_LIST.at(BOOTCODE2STRING));


}






































