#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "defaultBootloaderCode.h"
#include "defaultFlashDriverCode.h"
#include "crc16.h"
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

    //初始化控件
    componentsInitialization();
    //设置布局
    layoutsInitialization();


    //crc test code

//    {
//        QString filePathName = QFileDialog::getOpenFileName();
//        QString filePath = QFileInfo(filePathName).absolutePath();

//        qDebug()<<filePathName<<endl<<filePath<<endl;

//        if(filePathName.isEmpty())
//        {
//            QMessageBox::warning(this, "Warnning", "generate failed, please select a file", QMessageBox::Yes);
//            return;
//        }

//        QFile targetFile(filePathName);
//        if(!targetFile.open(QIODevice::ReadOnly | QIODevice::Text))
//        {
//            QMessageBox::warning(this, "Warnning", "Cannot open " + filePathName, QMessageBox::Yes);
//            return;
//        }

//        QTextStream targetFileIn(&targetFile);
//        QStringList targetCodeStringList;
//        while(!targetFileIn.atEnd())
//        {
//            QString readStr = targetFileIn.readLine();

//            if(!readStr.isEmpty())
//                targetCodeStringList.push_back(readStr);
//        }

//        targetFile.close();

//        QString file = targetCodeStringList.join('\n');

//        unsigned int crc = calcCRC(file.size(), file);

//        qDebug() << crc << "----" << ((crc>>8)&0xff) <<"----"<< (crc&0xff) <<endl;
//    }
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
        m_gbBootloader->setVisible(true);
        m_gbS19Selector->setVisible(true);
        m_gbDiagnosis->setVisible(false);
        break;
    case DIAGNOSIS:
        m_btnGenerate->setStatusTip(tr("generate firmware for diagnosis"));
        m_btnGenerate->setText(tr("generate"));
        m_gbBootloader->setVisible(false);
        m_gbS19Selector->setVisible(true);
        m_gbDiagnosis->setVisible(true);
        break;
    case BOOTCODE2STRING:
        m_btnGenerate->setStatusTip(tr("convert code to a char array"));
        m_btnGenerate->setText(tr("load and generate"));
        m_gbBootloader->setVisible(false);
        m_gbS19Selector->setVisible(false);
        m_gbDiagnosis->setVisible(false);
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

    qDebug()<<"height:"<<this->size().height();
    qDebug()<<"width:"<<this->size().width();
}

void MainWindow::useDefaultBootloaderPressed()
{
    if(m_ckbUseDefaultBootloader->isChecked())
    {
        m_btnLoadBootloader->setEnabled(false);
        m_leBootloaderInfo->setText(tr("default boot code is loaded"));
        m_leBootloaderInfo->setEnabled(false);
        m_btnLoadBootloader->setStatusTip(tr("use default boot code now"));
    }
    else
    {
        m_btnLoadBootloader->setEnabled(true);
        m_leBootloaderInfo->setEnabled(true);
        m_leBootloaderInfo->clear();
        m_btnLoadBootloader->setStatusTip(tr("select the bootloader file"));
    }
}

void MainWindow::loadBootloaderPressed()
{
    QString fileName = QFileDialog::getOpenFileName();
    m_leBootloaderInfo->setText(fileName);

    qDebug()<<fileName<<endl;
}

void MainWindow::s021ReturnedPressed()
{
    QString originalS021Data = m_leDiagnosisS021->text();

    if(originalS021Data.isEmpty())
    {
        QMessageBox::warning(this, "Warnning", "empty data, couldn't modify version", QMessageBox::Yes);
        return;
    }

    //S021***30302E30312E323040, 至少也要有22个有效字节(实际要更多，这里先这样校验即可)
    if(!originalS021Data.startsWith("S021", Qt::CaseInsensitive) || originalS021Data.size() < 22)
    {
        QMessageBox::warning(this, "Warnning", "invalid data, couldn't modify version", QMessageBox::Yes);
        return;
    }

    //请求用户输入版本信息数据
    bool isOK;
    QString versionQueryData = QInputDialog::getText(NULL,
                                                     "version data query",
                                                     "Please input new version, format: xx.xx.xx\n",
                                                     QLineEdit::Normal,
                                                     "",
                                                     &isOK);
    //校验输入信息, 严格匹配xx.xx.xx
    QRegExp regExp("^([a-fA-F\\d]{2}\\.){2}[a-fA-F\\d]{2}$");

    if(isOK && regExp.exactMatch(versionQueryData))
    {
        QString newS021String = originalS021Data.left(originalS021Data.size() - 18);
        newS021String += versionQueryData.toLatin1().toHex();
        newS021String += originalS021Data.right(2);

        m_leDiagnosisS021->setText(newS021String.toUpper());
    }
    else
    {
        QMessageBox::warning(this, "Warnning", "invalid version data, version update failed", QMessageBox::Yes);
        return;
    }
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
    QStringList originalS19FileStringList;
    while(!s19FileIn.atEnd())
    {
        QString readStr = s19FileIn.readLine();

        if(!readStr.isEmpty())
            originalS19FileStringList.push_back(readStr);
    }
    s19File.close();

    for(auto& elem:originalS19FileStringList)
        elem += '\n';

    //将bootloader合成进原始S19文件
    int targetIndex = originalS19FileStringList.indexOf(TARGET_STRING_AFTER_GENERATING_BOOTCODE);
    if(-1 != targetIndex)
    {
        QMessageBox::warning(this, "Warnning", "please check the .S19 file, maybe bootloader is already exist", QMessageBox::Yes);
        return;
    }

    int replaceIndex = originalS19FileStringList.indexOf(REPLACE_STRING);
    if(-1 == replaceIndex)
    {
        QMessageBox::warning(this, "Warnning", "please check the .S19 file, interrupt vector table line doesn't exist", QMessageBox::Yes);
        return;
    }
    originalS19FileStringList.replace(replaceIndex, bootloaderCodeString);

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
    for(auto elem:originalS19FileStringList)
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

//生成诊断仪用flash driver
void MainWindow::generateFlashDriverForDiagnosis(QString dir_path)
{
    QString filePathName = dir_path + "CherryT19SeriesFlashDriver.S19";

    QDir dir(dir_path);
    if(!dir.exists())
        dir.mkdir(dir_path);

    //删除临时文件
    QFile tmpFile(filePathName);
    if (tmpFile.exists())
    {
        tmpFile.remove();
    }

    QFile newFile(filePathName);
    if(!newFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QMessageBox::warning(this, "Warnning", "Cannot open " + filePathName, QMessageBox::Yes);
        return;
    }

    QTextStream out(&newFile);
    out << DEFAULT_FLASHDRIVER_CODE;

    newFile.close();
}

//生成诊断仪用固件
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
        if(!(m_leDiagnosisS021->text().startsWith("S021", Qt::CaseInsensitive)))
        {
            QMessageBox::warning(this, "Warnning", "Please check S021 data", QMessageBox::Yes);
            return;
        }
    }

    //获取.S19原文件
    QString fileName =fileInfo.at(ABSOLUTE_FILE_PATH);
    QFile s19File(fileName);
    if(!s19File.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::warning(this, "Warnning", "Cannot open " + fileName, QMessageBox::Yes);
        return;
    }
    QTextStream s19FileIn(&s19File);
    QStringList originalS19FileStringList;
    while(!s19FileIn.atEnd())
    {
        QString readStr = s19FileIn.readLine();

        if(!readStr.isEmpty())
            originalS19FileStringList.push_back(readStr);
    }
    s19File.close();

    for(auto& elem:originalS19FileStringList)
        elem += '\n';

    //校验原文件是否正确
    int bootValidateIndex = originalS19FileStringList.indexOf(TARGET_STRING_AFTER_GENERATING_BOOTCODE);
    if(-1 != bootValidateIndex)
    {
        QMessageBox::warning(this, "Warnning", "please check the .S19 file, bootloader code exists", QMessageBox::Yes);
        return;
    }

    if(originalS19FileStringList.first().startsWith("S021", Qt::CaseInsensitive))
    {
        QMessageBox::warning(this, "Warnning", "please check the .S19 file, S021 code is already exist in first line", QMessageBox::Yes);
        return;
    }

    if(originalS19FileStringList.at(originalS19FileStringList.size() - 1).startsWith("S20CFE", Qt::CaseInsensitive))
    {
        QMessageBox::warning(this, "Warnning", "please check the .S19 file, CRC code is already exist in first line", QMessageBox::Yes);
        return;
    }

    int vectorValidateIndex = originalS19FileStringList.indexOf(REPLACE_STRING);
    if(-1 == vectorValidateIndex)
    {
        QMessageBox::warning(this, "Warnning", "please check the .S19 file, interrupt vector table S10B line doesn't exist", QMessageBox::Yes);
        return;
    }

    //替换S0行，删除S10B行
    originalS19FileStringList.replace(0, m_leDiagnosisS021->text() + '\n');
    originalS19FileStringList.removeOne(REPLACE_STRING);

    for(auto elem: originalS19FileStringList)
        qDebug() << elem << endl;

    //.S19文件中S2按Fx升序排序
    if(!sortS19Code(originalS19FileStringList))
    {
        return;
    }

    //生成临时文件
    QString tmpFileName = fileInfo.at(FILE_NAME);
    tmpFileName = tmpFileName.left(tmpFileName.size() - 4);
    tmpFileName += "_diagnosis(tmp).S19";

    QString folderName = "/generatedFirmwaresForDiagnosis/";
    QString dirPath = fileInfo.at(ABSOLUTE_PATH) + folderName;
    QDir dir(dirPath);
    if(!dir.exists())
        dir.mkdir(dirPath);

    QString tmpFilePathName = dirPath + tmpFileName;
    QFile newFile(tmpFilePathName);
    if(!newFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QMessageBox::warning(this, "Warnning", "Cannot open " + tmpFilePathName, QMessageBox::Yes);
        return;
    }

    QTextStream out(&newFile);
    for(auto elem:originalS19FileStringList)
    {
        if(elem != "\n")
            out << elem;
    }
    newFile.close();

    //清空上次输入的S20CFE数据
    m_leDiagnosisS20C->clear();

    //请求用户输入S20CFE数据
    bool isOK;
    QString s20cQuery = QInputDialog::getText(NULL, "CRC data query",
                                               "Please input CRC result code starts with S20CFE, using file:\n" + tmpFilePathName,
                                               QLineEdit::Normal,
                                               "",
                                               &isOK);

    if(isOK && s20cQuery.startsWith("S20CFE", Qt::CaseInsensitive))
    {
        s20cQuery = s20cQuery.toUpper();
        m_leDiagnosisS20C->setText(s20cQuery);
        originalS19FileStringList.insert(originalS19FileStringList.size() - 1, s20cQuery + '\n');

        //删除临时文件
        QFile tmpFile(tmpFilePathName);
        if (tmpFile.exists())
        {
            tmpFile.remove();
        }

        //生成固件并在文件夹中定位此文件
        QString newFilePathName = tmpFilePathName.left(tmpFilePathName.size() - 9);
        QString timeInfo = QDateTime::currentDateTime().toString("yyyy-MM-dd_HH-mm-ss");
        newFilePathName += "_" + timeInfo + ".S19";

        QFile newFile(newFilePathName);
        if(!newFile.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            QMessageBox::warning(this, "Warnning", "Cannot open " + newFilePathName, QMessageBox::Yes);
            return;
        }

        QTextStream out(&newFile);
        for(auto elem:originalS19FileStringList)
        {
            if(elem != "\n")
                out << elem;
        }
        newFile.close();

        //同时生成一个flash driver
        generateFlashDriverForDiagnosis(dirPath);
#ifdef WIN32
        QProcess process;
        QString openNewFileName = newFilePathName;

        openNewFileName.replace("/", "\\");    //***这句windows下必要***
        process.startDetached("explorer /select," + openNewFileName);
#endif
    }
    else
    {
        QMessageBox::warning(this, "Warnning", "please input correct CRC data", QMessageBox::Yes);

        //删除临时文件
        QFile tmpFile(tmpFilePathName);
        if (tmpFile.exists())
        {
            tmpFile.remove();
        }

        return;
    }
}

bool MainWindow::sortS19Code(QStringList &originalStringList)
{
    QStringList stringListS0AndS9;
    QStringList stringListS1;
    QStringList stringListS2[16];

    //S0、S9行
    stringListS0AndS9.push_back(originalStringList.first().toUpper());
    stringListS0AndS9.push_back(originalStringList.last());

    for(QString elem: originalStringList)
    {
        //S1代码段
        if(elem.startsWith("S1", Qt::CaseInsensitive))
        {
            stringListS1.push_back(elem);
        }

        if(elem.startsWith("S2", Qt::CaseInsensitive))
        {
            //S2代码段，开头为：S2**Fx, x在0-F之间
            int index = hexCharToHex(elem.at(5).toLatin1());

            if(index >= 0 && index < 16)
            {
                stringListS2[index].push_back(elem);
            }
            else
            {
                QMessageBox::warning(this, "Warnning", "check:" + elem, QMessageBox::Yes);
                return false;
            }

        }
    }

    //重组originalStringList,按S0-S1-S2-S9的顺序
    originalStringList.clear();
    originalStringList.push_front(stringListS0AndS9.first());
    originalStringList += stringListS1;
    for(int cnt = 0; cnt < 16; ++cnt)
    {
        if(!stringListS2[cnt].isEmpty())
            originalStringList += stringListS2[cnt];
    }
    originalStringList.push_back(stringListS0AndS9.last());

    return true;
}

int MainWindow::hexCharToHex(char src)
{
    int ret = -1;
    src = toupper(src);

    if(src >= '0' && src <= '9')
    {
        ret = src - '0';
    }
    else if(src >= 'A' && src <= 'F')
    {
        ret = src - 'A' + 10;
    }

    return ret;
}

//计算CRC，参考《ECU bootloader and programming implementation specification》
unsigned int MainWindow::calcCRC(unsigned int size, QString fileData)
{
    unsigned int crc = 0xffff; /* initial value */
    unsigned char tmp = 0;
    unsigned int i = 0;

    QByteArray charArray = fileData.toLatin1();

    for(i = 0; i < size; i++)
    {
        if('\n' != charArray.at(i))
        {
            tmp=(crc>>8)^charArray.at(i);
            crc=(crc<<8)^crcLookupTable[tmp];
        }
    }

    return crc;
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

void MainWindow::componentsInitialization(void)
{
    setWindowTitle(tr("CherryT19SeriesFirmwareGenerator"));

    //窗体名称及状态栏设置
    auto labelAuthorInfo = new QLabel;
    labelAuthorInfo->setStatusTip(tr("click to view source code on github"));
    labelAuthorInfo->setOpenExternalLinks(true);
    labelAuthorInfo->setText(QString::fromLocal8Bit("<style> a {text-decoration: none} </style> <a href = https://www.github.com/bingshuizhilian/QTPROJECTS> contact author </a>"));
    labelAuthorInfo->show();
    ui->statusBar->addPermanentWidget(labelAuthorInfo);

    //选择.S19文件按钮
    m_btnChooseFile = new QPushButton(tr("load file"));
    connect(m_btnChooseFile, &m_btnChooseFile->clicked, this, &selectFilePressed);
    m_btnChooseFile->setStatusTip(tr("select the target .S19 file"));
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
    m_ckbUseDefaultBootloader = new QCheckBox(tr("default"));
    connect(m_ckbUseDefaultBootloader, &m_ckbUseDefaultBootloader->stateChanged, this, &useDefaultBootloaderPressed);
    m_ckbUseDefaultBootloader->setStatusTip(tr("use default boot code when checked"));
    m_ckbUseDefaultBootloader->setChecked(true);

    //生成按钮
    m_btnGenerate = new QPushButton(tr("generate"));
    connect(m_btnGenerate, &m_btnGenerate->clicked, this, &generateButtonPressed);

    //合成诊断仪firmware时需要的额外信息
    m_leDiagnosisS021 = new QLineEdit;
    connect(m_leDiagnosisS021, &m_leDiagnosisS021->returnPressed, this, &s021ReturnedPressed);
    m_leDiagnosisS021->setStatusTip("press enter to modify version");
    m_leDiagnosisS20C = new QLineEdit;
    m_leDiagnosisS20C->setReadOnly(true);

    //布局控件
    m_gbBootloader = new QGroupBox;
    m_gbBootloader->setTitle(tr("bootloader settings"));
    m_gbS19Selector = new QGroupBox;
    m_gbS19Selector->setTitle(tr("select .S19 file"));
    m_gbDiagnosis = new QGroupBox;
    m_gbDiagnosis->setTitle(tr("diagnosis settings"));

    //功能选择下拉框
    m_cmbFunctionSwitch = new QComboBox;
    m_cmbFunctionSwitch->setStatusTip("select a function");
    connect(m_cmbFunctionSwitch,  static_cast<void (QComboBox::*)(const QString&)>(&QComboBox::currentTextChanged), this, &switchFunctionPressed);
    m_cmbFunctionSwitch->addItem(FUNCTION_STRING_LIST.at(BOOTLOADER), FUNCTION_STRING_LIST.at(BOOTLOADER));
    m_cmbFunctionSwitch->addItem(FUNCTION_STRING_LIST.at(DIAGNOSIS), FUNCTION_STRING_LIST.at(DIAGNOSIS));
    m_cmbFunctionSwitch->addItem(FUNCTION_STRING_LIST.at(BOOTCODE2STRING), FUNCTION_STRING_LIST.at(BOOTCODE2STRING));

}

void MainWindow::layoutsInitialization()
{
    //bootloader控件区
    auto layoutBootloader = new QGridLayout;
    auto labelLeBootloader = new QLabel(tr("bootloader"));
    layoutBootloader->addWidget(labelLeBootloader, 0, 0);
    layoutBootloader->addWidget(m_leBootloaderInfo, 0, 1);
    layoutBootloader->addWidget(m_ckbUseDefaultBootloader, 1, 0);
    layoutBootloader->addWidget(m_btnLoadBootloader, 1, 1);
    m_gbBootloader->setLayout(layoutBootloader);

    //select file控件区
    auto layoutS19Selector = new QGridLayout;
    layoutS19Selector->addWidget(m_btnChooseFile, 0, 0);
    layoutS19Selector->addWidget(m_leFileInfo, 0, 1);
    m_gbS19Selector->setLayout(layoutS19Selector);

    //diagnosis控件区
    auto layoutDiagnosis = new QGridLayout;
    auto labelLeS021 = new QLabel(tr("S021:"));
    labelLeS021->setStatusTip("the first line of the firmware");
    auto labelLeS20C = new QLabel(tr("S20C:"));
    labelLeS20C->setStatusTip("the second last line of the firmware");
    layoutDiagnosis->addWidget(labelLeS021, 0, 0);
    layoutDiagnosis->addWidget(m_leDiagnosisS021, 0, 1);
    layoutDiagnosis->addWidget(labelLeS20C, 1, 0);
    layoutDiagnosis->addWidget(m_leDiagnosisS20C, 1, 1);
    m_gbDiagnosis->setLayout(layoutDiagnosis);

    //globle layout
    m_layoutGlobal = new QVBoxLayout;
    m_layoutGlobal->addWidget(m_cmbFunctionSwitch);
    m_layoutGlobal->addWidget(m_gbBootloader);
    m_layoutGlobal->addWidget(m_gbS19Selector);
    m_layoutGlobal->addWidget(m_gbDiagnosis);
    m_layoutGlobal->addWidget(m_btnGenerate);

    ui->centralWidget->setLayout(m_layoutGlobal);

    this->resize(QSize(WINDOW_WIDTH, WINDOW_HEIGHT));
    this->setFixedHeight(WINDOW_HEIGHT);
}
