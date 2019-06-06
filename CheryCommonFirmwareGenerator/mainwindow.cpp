#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "defaultM1SeriesBootCode.h"
#include "defaultT1SeriesBootCode.h"
#include "defaultS51evflBootCode.h"
#include "defaultA13tevflBootCode.h"
#include "defaultFlashDriverCode.h"
#include "defaultEraseEepromCode.h"
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
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QJsonParseError>
#include <QStandardPaths>
#include <QClipboard>
#include <QUrl>
#include <QProgressBar>
#include <QDesktopServices>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
ui->menuBar->show();
    //初始化控件
    componentsInitialization();
    //设置布局
    layoutsInitialization();
    //命令行初始化
    commandsInitialization();

    //升级检测
    procConfigFile(CMD_LOAD_CONFIG_FILE);
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
        if(this->isFullScreen())
            this->showNormal();
        this->setFixedSize(WINDOW_WIDTH, WINDOW_HEIGHT);
        m_cmbFunctionSwitch->setFixedWidth(182);
        m_gbSwitchFunction->setTitle(tr("switch function"));
        m_leRunCommand->setVisible(false);
        m_btnGenerate->setStatusTip(tr("generate firmware with bootloader"));
        m_btnGenerate->setVisible(true);
        m_gbBootloader->setVisible(true);
        m_gbS19Selector->setVisible(true);
        m_gbDiagnosis->setVisible(false);
        ptOutputWnd->setVisible(false);
        break;
    case DIAGNOSIS:
        if(this->isFullScreen())
            this->showNormal();
        this->setFixedSize(WINDOW_WIDTH, WINDOW_HEIGHT);
        m_cmbFunctionSwitch->setFixedWidth(182);
        m_gbSwitchFunction->setTitle(tr("switch function"));
        m_leRunCommand->setVisible(false);
        m_btnGenerate->setStatusTip(tr("generate firmware for diagnosis"));
        m_btnGenerate->setVisible(true);
        m_gbBootloader->setVisible(false);
        m_gbS19Selector->setVisible(true);
        m_gbDiagnosis->setVisible(true);
        ptOutputWnd->setVisible(false);
        break;
    case CMD_HANDLER:
        qDebug() << this->windowFlags();
        this->setFixedSize(QSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX));
        this->setMinimumSize(WINDOW_WIDTH * 3 / 2, WINDOW_HEIGHT * 3 / 2);
        m_cmbFunctionSwitch->setFixedWidth(90);
        m_gbSwitchFunction->setTitle(tr("input command"));
        m_leRunCommand->setVisible(true);
        m_btnGenerate->setVisible(false);
        m_gbBootloader->setVisible(false);
        m_gbS19Selector->setVisible(false);
        m_gbDiagnosis->setVisible(false);
        ptOutputWnd->setVisible(true);
        break;
    default:
        break;
    }
}

//处理命令输入行按回车执行命令事件
void MainWindow::runCmdReturnPressed()
{
    QString inputString = m_leRunCommand->text();
    CmdType findCmd = CMD_MAX;

    if(inputString.startsWith("::"))
    {
        findCmd = CMD_WINDOWS_COMMON;
    }
    else
    {
        foreach(auto pairElem, cmdList)
        {
            foreach(auto strListElem, pairElem.second)
            {
                if(!strListElem.compare(inputString, Qt::CaseInsensitive))
                {
                    findCmd = pairElem.first;
                    break;
                }
            }

            if(CMD_MAX != findCmd)
                break;
        }
    }

    switch(findCmd)
    {
    case CMD_HELP:
    case CMD_HELP_BOOTLOADER:
    case CMD_HELP_DIAGNOSIS:
    case CMD_HELP_DIAG_CALCULATE_KEY:
        showHelpInfo(findCmd);
        break;
    case CMD_CLEAR_SCREEN:
        ptOutputWnd->clear();
        break;
    case CMD_FULL_SCREEN:
        this->showFullScreen();
        break;
    case CMD_NORMAL_SCREEN:
        this->showNormal();
        break;
    case CMD_QUIT_APPLICATION:
        QApplication::quit();
        break;
    case CMD_SAVE_CONFIG_FILE:
    case CMD_LOAD_CONFIG_FILE:
        procConfigFile(findCmd);
        break;
    case CMD_CODE_TO_STRING:
        generateCharArray();
        break;
    case CMD_COMPRESS_BMP:
        compressCArrayOfBitmap();
        break;
    case CMD_GEN_M1AFL2_FLASH_DRIVER:
    {
        QString dirPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
        dirPath.append("/cheryM1afl2FlashDriver/");
        ptOutputWnd->clear();
        ptOutputWnd->appendPlainText(dirPath.left(dirPath.size() - 1));
        generateFiles(findCmd, dirPath, true);
        break;
    }
    case CMD_GEN_T19_FLASH_DRIVER:
    {
        QString dirPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
        dirPath.append("/cheryT19FlashDriver/");
        ptOutputWnd->clear();
        ptOutputWnd->appendPlainText(dirPath.left(dirPath.size() - 1));
        generateFiles(findCmd, dirPath, true);
        break;
    }
    case CMD_GEN_S51EVFL_FLASH_DRIVER:
    {
        QString dirPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
        dirPath.append("/cheryS51evflFlashDriver/");
        ptOutputWnd->clear();
        ptOutputWnd->appendPlainText(dirPath.left(dirPath.size() - 1));
        generateFiles(findCmd, dirPath, true);
        break;
    }
    case CMD_GEN_A13TEV_FLASH_DRIVER:
    {
        QString dirPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
        dirPath.append("/cheryA13tevFlashDriver/");
        ptOutputWnd->clear();
        ptOutputWnd->appendPlainText(dirPath.left(dirPath.size() - 1));
        generateFiles(findCmd, dirPath, true);
        break;
    }
    case CMD_GEN_COMMON_FLASH_DRIVER:
    {
        QString dirPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
        dirPath.append("/cheryCommonFlashDriver/");
        ptOutputWnd->clear();
        ptOutputWnd->appendPlainText(dirPath.left(dirPath.size() - 1));
        generateFiles(findCmd, dirPath, true);
        break;
    }
    case CMD_GEN_ERASE_EEPROM:
    {
        QString dirPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
        dirPath.append("/cheryM1SeriesEraseEepromFirmware/");
        ptOutputWnd->clear();
        ptOutputWnd->appendPlainText(dirPath.left(dirPath.size() - 1));
        generateFiles(findCmd, dirPath, true);
        break;
    }
    case CMD_GEN_M1_BOOT_CODE:
    {
        QString dirPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
        dirPath.append("/cheryM1SeriesBootCode/");
        ptOutputWnd->clear();
        ptOutputWnd->appendPlainText(dirPath.left(dirPath.size() - 1));
        generateFiles(findCmd, dirPath, true);
        break;
    }
    case CMD_GEN_T1_BOOT_CODE:
    {
        QString dirPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
        dirPath.append("/cheryT1SeriesBootCode/");
        ptOutputWnd->clear();
        ptOutputWnd->appendPlainText(dirPath.left(dirPath.size() - 1));
        generateFiles(findCmd, dirPath, true);
        break;
    }
    case CMD_GEN_S51EVFL_BOOT_CODE:
    {
        QString dirPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
        dirPath.append("/cheryS51evflBootCode/");
        ptOutputWnd->clear();
        ptOutputWnd->appendPlainText(dirPath.left(dirPath.size() - 1));
        generateFiles(findCmd, dirPath, true);
        break;
    }
    case CMD_GEN_A13TEV_BOOT_CODE:
    {
        QString dirPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
        dirPath.append("/cheryA13tevBootCode/");
        ptOutputWnd->clear();
        ptOutputWnd->appendPlainText(dirPath.left(dirPath.size() - 1));
        generateFiles(findCmd, dirPath, true);
        break;
    }
    case CMD_DIAG_CALCULATE_KEY:
        dealWithCalculateKeyCommand();
        break;
#if WIN32
    case CMD_WINDOWS_COMMON:
    {
        QString windowsCmd = inputString.right(inputString.size() - 2);
        bool isSuccess = QProcess::startDetached(windowsCmd);
        QString outputResult;

        if(isSuccess)
        {
            outputResult = windowsCmd + " -> success";
        }
        else
        {
            if(!windowsCmd.isEmpty())
                outputResult = windowsCmd + " -> fail";
            else
                outputResult = "please input a command";
        }

        ptOutputWnd->appendPlainText(outputResult);
        break;
    }
    case CMD_WINDOWS_CALCULATOR:
    case CMD_WINDOWS_TASKMANAGER:
    case CMD_WINDOWS_PAINT:
    {
        QString windowsCmd = cmdList.at(findCmd).second.first();
        windowsCmd = windowsCmd.right(windowsCmd.size() - 1);
        QProcess::startDetached(windowsCmd);
        break;
    }
#endif
    default:
        ptOutputWnd->clear();
        ptOutputWnd->appendHtml("Unknown command, input <u>:?</u> for help.");
        break;
    }
}

void MainWindow::switchPlatformPressed()
{
    m_leBootloaderInfo->setText("default " + m_cmbPlatformSwitch->currentText() + " boot code loaded");

    if(PLATFORM_STRING_LIST.at(M1AFL2) == m_cmbPlatformSwitch->currentText())
    {
        m_leBootloaderInfo->setStatusTip(QString("HW VER in current bootloader : ") + tr(DEFAULT_M1_SERIES_BOOT_CODE_HARDWARE_VERSION));
    }
    else if(PLATFORM_STRING_LIST.at(T18) == m_cmbPlatformSwitch->currentText())
    {
        m_leBootloaderInfo->setStatusTip(QString("HW VER in current bootloader : ") + tr(DEFAULT_T1_SERIES_BOOT_CODE_HARDWARE_VERSION));
    }
    else if(PLATFORM_STRING_LIST.at(T19) == m_cmbPlatformSwitch->currentText())
    {
        m_leBootloaderInfo->setStatusTip(QString("HW VER in current bootloader : ") + tr(DEFAULT_T1_SERIES_BOOT_CODE_HARDWARE_VERSION));
    }
    else if(PLATFORM_STRING_LIST.at(S51EVFL) == m_cmbPlatformSwitch->currentText())
    {
        m_leBootloaderInfo->setStatusTip(QString("HW VER in current bootloader : ") + tr(DEFAULT_S51EVFL_BOOT_CODE_HARDWARE_VERSION));
    }
    else if(PLATFORM_STRING_LIST.at(A13TEV) == m_cmbPlatformSwitch->currentText())
    {
        m_leBootloaderInfo->setStatusTip(QString("HW VER in current bootloader : ") + tr(DEFAULT_A13TEV_BOOT_CODE_HARDWARE_VERSION));
    }
}

//处理按下generate按钮事件
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
    default:
        break;
    }

    qDebug()<<"wd height:"<<this->size().height();
    qDebug()<<"wd width:"<<this->size().width();
}

//处理使用默认bootloader事件
void MainWindow::useDefaultBootloaderPressed()
{
    if(m_ckbUseDefaultBootloader->isChecked())
    {
        m_btnLoadBootloader->setEnabled(false);
        m_leBootloaderInfo->setText("default " + m_cmbPlatformSwitch->currentText() + " boot code loaded");
        m_leBootloaderInfo->setEnabled(false);
        m_btnLoadBootloader->setStatusTip(tr("use default boot code now"));
        m_cmbPlatformSwitch->setEnabled(true);

        if(PLATFORM_STRING_LIST.at(M1AFL2) == m_cmbPlatformSwitch->currentText())
        {
            m_leBootloaderInfo->setStatusTip(QString("HW VER in current bootloader : ") + tr(DEFAULT_M1_SERIES_BOOT_CODE_HARDWARE_VERSION));
        }
        else if(PLATFORM_STRING_LIST.at(T18) == m_cmbPlatformSwitch->currentText())
        {
            m_leBootloaderInfo->setStatusTip(QString("HW VER in current bootloader : ") + tr(DEFAULT_T1_SERIES_BOOT_CODE_HARDWARE_VERSION));
        }
        else if(PLATFORM_STRING_LIST.at(T19) == m_cmbPlatformSwitch->currentText())
        {
            m_leBootloaderInfo->setStatusTip(QString("HW VER in current bootloader : ") + tr(DEFAULT_T1_SERIES_BOOT_CODE_HARDWARE_VERSION));
        }
        else if(PLATFORM_STRING_LIST.at(S51EVFL) == m_cmbPlatformSwitch->currentText())
        {
            m_leBootloaderInfo->setStatusTip(QString("HW VER in current bootloader : ") + tr(DEFAULT_S51EVFL_BOOT_CODE_HARDWARE_VERSION));
        }
        else if(PLATFORM_STRING_LIST.at(A13TEV) == m_cmbPlatformSwitch->currentText())
        {
            m_leBootloaderInfo->setStatusTip(QString("HW VER in current bootloader : ") + tr(DEFAULT_A13TEV_BOOT_CODE_HARDWARE_VERSION));
        }
    }
    else
    {
        m_btnLoadBootloader->setEnabled(true);
        m_leBootloaderInfo->setEnabled(true);
        m_leBootloaderInfo->clear();
        m_btnLoadBootloader->setStatusTip(tr("select the bootloader file"));
        m_cmbPlatformSwitch->setEnabled(false);
        m_leBootloaderInfo->setStatusTip(tr(""));
    }
}

//处理加载bootloader事件
void MainWindow::loadBootloaderPressed()
{
    QString fileName = QFileDialog::getOpenFileName();

    if(!fileName.isEmpty())
        m_leBootloaderInfo->setText(fileName);

    qDebug()<<fileName<<endl;
}

//处理在S021输入框中按回车键修改版本号或输入零件号事件
void MainWindow::s021ReturnPressed()
{
    QString originalS021Data = m_leDiagnosisS021->text();

    if(originalS021Data.isEmpty())
    {
        QString tmpStr = DIAG_COMMON_S0;

        //请求用户输入零件号
        bool isOK;
        QString partnumberQueryData = QInputDialog::getText(NULL,
                                                         "part number query",
                                                         "Please input part number, up to 16 characters\n",
                                                         QLineEdit::Normal,
                                                         "",
                                                         &isOK);
        //校验输入信息
        QRegExp regExp("^[\\w\\-]{0,16}$");

        if(isOK && regExp.exactMatch(partnumberQueryData))
        {
            tmpStr.replace(18, partnumberQueryData.toLatin1().toHex().size(), partnumberQueryData.toLatin1().toHex());
            m_leDiagnosisS021->setText(tmpStr.toUpper());
        }
        else
        {
            QMessageBox::warning(this, "Warnning", "invalid part number", QMessageBox::Yes);
            return;
        }
    }

    if(":M" == m_leDiagnosisS021->text() || ":m" == m_leDiagnosisS021->text())
    {
        QString tmpStr = DIAG_COMMON_S0;
        tmpStr.replace(18, DIAG_M1AFL2_PARTNUMBER.toLatin1().toHex().size(), DIAG_M1AFL2_PARTNUMBER.toLatin1().toHex());
        m_leDiagnosisS021->setText(tmpStr.toUpper());
    }

    if(":t" == m_leDiagnosisS021->text() || ":T" == m_leDiagnosisS021->text())
    {
        QString tmpStr = DIAG_COMMON_S0;
        tmpStr.replace(18, DIAG_T19_PARTNUMBER.toLatin1().toHex().size(), DIAG_T19_PARTNUMBER.toLatin1().toHex());
        m_leDiagnosisS021->setText(tmpStr.toUpper());
    }

    if(":s" == m_leDiagnosisS021->text() || ":S" == m_leDiagnosisS021->text())
    {
        QString tmpStr = DIAG_COMMON_S0;
        tmpStr.replace(18, DIAG_S51EVFL_PARTNUMBER.toLatin1().toHex().size(), DIAG_S51EVFL_PARTNUMBER.toLatin1().toHex());
        m_leDiagnosisS021->setText(tmpStr.toUpper());
    }

    if(":a" == m_leDiagnosisS021->text() || ":A" == m_leDiagnosisS021->text())
    {
        QString tmpStr = DIAG_COMMON_S0;
        tmpStr.replace(18, DIAG_A13TEV_PARTNUMBER.toLatin1().toHex().size(), DIAG_A13TEV_PARTNUMBER.toLatin1().toHex());
        m_leDiagnosisS021->setText(tmpStr.toUpper());
    }

    originalS021Data = m_leDiagnosisS021->text();

    //S021***30302E30312E323040
    if(!originalS021Data.startsWith("S021", Qt::CaseInsensitive) || m_leDiagnosisS021->text().size() != 70)
    {
        QMessageBox::warning(this, "Warnning", "invalid data, couldn't modify version", QMessageBox::Yes);
        return;
    }

    //请求用户输入版本信息数据
    bool isOK;
    QString versionQueryData = QInputDialog::getText(NULL,
                                                     "version data query",
                                                     "Please input version info, format: xx.xx.xx\n",
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

//处理加载.S19文件事件
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

//合成含bootloader的固件
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

        for(QString& elem:bootloaderCodeStringList)
        {
            elem += '\n';

            //原生的bootloader.S19文件有文件头和文件尾，合成的时候需要剔除
            if(elem.startsWith("S0", Qt::CaseInsensitive))
                QMessageBox::information(this, "Tips", "S0 line detected in the bootloader file, the tool will ignore it and will not merge it into the target file", QMessageBox::Yes);
            else if(elem.startsWith("S9", Qt::CaseInsensitive))
                QMessageBox::information(this, "Tips", "S9 line detected in the bootloader file, the tool will ignore it and will not merge it into the target file", QMessageBox::Yes);
            else
                bootloaderCodeString += elem;
        }

        int targetIndex = bootloaderCodeStringList.indexOf(TARGET_STRING_AFTER_GENERATING_BOOTCODE);
        if(-1 == targetIndex)
        {
            QMessageBox::warning(this, "Warnning", "please check the bootloader file, interrupt vector table line doesn't exist", QMessageBox::Yes);
            return;
        }
    }
    else
    {
        if(PLATFORM_STRING_LIST.at(M1AFL2) == m_cmbPlatformSwitch->currentText())
        {
            bootloaderCodeString = DEFAULT_M1_SERIES_BOOT_CODE;
        }
        else if(PLATFORM_STRING_LIST.at(T18) == m_cmbPlatformSwitch->currentText())
        {
            bootloaderCodeString = DEFAULT_T1_SERIES_BOOT_CODE;
        }
        else if(PLATFORM_STRING_LIST.at(T19) == m_cmbPlatformSwitch->currentText())
        {
            bootloaderCodeString = DEFAULT_T1_SERIES_BOOT_CODE;
        }
        else if(PLATFORM_STRING_LIST.at(S51EVFL) == m_cmbPlatformSwitch->currentText())
        {
            bootloaderCodeString = DEFAULT_S51EVFL_BOOT_CODE;
        }
        else if(PLATFORM_STRING_LIST.at(A13TEV) == m_cmbPlatformSwitch->currentText())
        {
            bootloaderCodeString = DEFAULT_A13TEV_BOOT_CODE;
        }
        else
        {
            qDebug() << "error occurred when loading default boot code";
        }
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
        QMessageBox::warning(this, "Warnning", "please check the .S19 file, bootloader code already exists", QMessageBox::Yes);
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
void MainWindow::generateFiles(CmdType cmd, QString dir_path, bool is_open_folder)
{
    QString filePathName = dir_path;
    const char *fileContent = nullptr;

    switch(cmd)
    {
    case CMD_GEN_M1AFL2_FLASH_DRIVER:
        filePathName += "CheryM1afl2FlashDriver.S19";
        fileContent = DEFAULT_M1AFL2_FLASHDRIVER_CODE;
        break;
    case CMD_GEN_T19_FLASH_DRIVER:
        filePathName += "CheryT19FlashDriver.S19";
        fileContent = DEFAULT_T19_FLASHDRIVER_CODE;
        break;
    case CMD_GEN_S51EVFL_FLASH_DRIVER:
        filePathName += "CheryS51evflFlashDriver.S19";
        fileContent = DEFAULT_S51EVFL_FLASHDRIVER_CODE;
        break;
    case CMD_GEN_A13TEV_FLASH_DRIVER:
        filePathName += "CheryA13tevFlashDriver.S19";
        fileContent = DEFAULT_A13TEV_FLASHDRIVER_CODE;
        break;
    case CMD_GEN_COMMON_FLASH_DRIVER:
        filePathName += "CheryCommonFlashDriver.S19";
        fileContent = DEFAULT_CHERY_COMMON_FLASHDRIVER_CODE;
        break;
    case CMD_GEN_ERASE_EEPROM:
        filePathName += "CheryM1SeriesEraseEepromFirmware.S19";
        fileContent = DEFAULT_ERASE_EEPROM_CODE;
        break;
    case CMD_GEN_M1_BOOT_CODE:
        filePathName += QString("CheryM1SeriesBootCode_hw") + DEFAULT_M1_SERIES_BOOT_CODE_HARDWARE_VERSION + QString(".S19");
        fileContent = DEFAULT_M1_SERIES_BOOT_CODE;
        break;
    case CMD_GEN_T1_BOOT_CODE:
        filePathName += QString("CheryT1SeriesBootCode_hw") + DEFAULT_T1_SERIES_BOOT_CODE_HARDWARE_VERSION + QString(".S19");
        fileContent = DEFAULT_T1_SERIES_BOOT_CODE;
        break;
    case CMD_GEN_S51EVFL_BOOT_CODE:
        filePathName += QString("CheryS51evflBootCode_hw") + DEFAULT_S51EVFL_BOOT_CODE_HARDWARE_VERSION + QString(".S19");
        fileContent = DEFAULT_S51EVFL_BOOT_CODE;
        break;
    case CMD_GEN_A13TEV_BOOT_CODE:
        filePathName += QString("CheryA13tevBootCode_hw") + DEFAULT_A13TEV_BOOT_CODE_HARDWARE_VERSION + QString(".S19");
        fileContent = DEFAULT_A13TEV_BOOT_CODE;
        break;
    default:
        break;
    }

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
    out << fileContent;


    newFile.close();

    if(is_open_folder)
    {
#ifdef WIN32
        QProcess process;
        QString openFileName = filePathName;

        openFileName.replace("/", "\\");    //***这句windows下必要***
        process.startDetached("explorer /select," + openFileName);
#endif
    }
}

//生成诊断仪用固件
void MainWindow::generateFirmwareForDiagnosis()
{
    if(m_leFileInfo->text().isEmpty())
    {
        QMessageBox::warning(this, "Warnning", "Please select a .S19 file", QMessageBox::Yes);
        return;
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

    //校验S021行数据
    if(m_leDiagnosisS021->text().isEmpty())
    {
        QMessageBox::warning(this, "Warnning", "Please input S021 data", QMessageBox::Yes);
        return;
    }
    else
    {
        if(!m_leDiagnosisS021->text().startsWith("S021", Qt::CaseInsensitive) || m_leDiagnosisS021->text().size() != 70)
        {
            QMessageBox::warning(this, "Warnning", "Please check S021 data, which starts with \"S021\", and the length should be equal to 70", QMessageBox::Yes);
            return;
        }
    }

    //替换S0行，删除S10B行
    originalS19FileStringList.replace(0, m_leDiagnosisS021->text().toUpper() + '\n');
    originalS19FileStringList.removeOne(REPLACE_STRING);

    for(auto elem: originalS19FileStringList)
        qDebug() << elem << endl;

    //.S19文件中S2按Fx升序排序
    if(!sortS19Code(originalS19FileStringList))
    {
        return;
    }

    //生成参与CRC计算的数据的字符串
    QStringList crcCalcStringList = originalS19FileStringList;
    QString crcCalcString;

    //S0和S9行不参与CRC计算
    if(crcCalcStringList.size() >= 2)
    {
        crcCalcStringList.pop_front();
        crcCalcStringList.pop_back();
    }

    for(QString &elem: crcCalcStringList)
    {
        if(elem.endsWith('\n'))
            elem = elem.left(elem.size() - 1);

        //最后两个字节不计算
        elem = elem.left(elem.size() - 2);

        //S1前8个字节不计算
        if(elem.startsWith("S1", Qt::CaseInsensitive))
            elem = elem.right(elem.size() - 8);

        //S2前10个字节不计算
        if(elem.startsWith("S2", Qt::CaseInsensitive))
            elem = elem.right(elem.size() - 10);

        crcCalcString += elem;
    }

    if(crcCalcString.size() % 2 != 0)
    {
        QMessageBox::warning(this, "Warnning", "data length of crc calculating error", QMessageBox::Yes);
        return;
    }

    //将数字字符串重组成数字，e.g. "91b0"->0x91 0xb0
    QList<unsigned char> dataList;
    for(int cnt = 0; cnt < crcCalcString.size(); cnt += 2)
        dataList.push_back(crcCalcString.mid(cnt, 2).toInt(nullptr, 16) & 0xff);

    unsigned short crc = calcCRC16(dataList);
    unsigned char chkSum = calcChecksum(crc);

    qDebug() << dataList.size();
    qDebug() << "crc: 0x" + QString::number(crc, 16);
    qDebug() << "crc high: 0x" + QString::number((crc >> 8) & 0xff, 16);
    qDebug() << "crc low: 0x" + QString::number(crc & 0xff, 16);
    qDebug() << "checksum: 0x" + QString::number(chkSum, 16);

    //S2 0C F48000 XX XX  XX XX  XX XX  XX XX  CHK
    QString s20cText;
    QString softwareVersion = QByteArray::fromHex(m_leDiagnosisS021->text().right(18).left(16).toLatin1());
    QString partNumber = m_leDiagnosisS021->text().right(m_leDiagnosisS021->text().size() - 18).left(32).remove("20");
    partNumber = QByteArray::fromHex(partNumber.toLatin1());

    if(m_leDiagnosisS021->text().contains(DIAG_M1AFL2_PARTNUMBER.toLatin1().toHex(), Qt::CaseInsensitive))
    {
        s20cText = "S20CFE8000";
    }
    else if(m_leDiagnosisS021->text().contains(DIAG_T19_PARTNUMBER.toLatin1().toHex(), Qt::CaseInsensitive))
    {
        s20cText = "S20CFE8000";
    }
    else if(m_leDiagnosisS021->text().contains(DIAG_S51EVFL_PARTNUMBER.toLatin1().toHex(), Qt::CaseInsensitive))
    {
        s20cText = "S20CFEBE00";
    }
    else if(m_leDiagnosisS021->text().contains(DIAG_A13TEV_PARTNUMBER.toLatin1().toHex(), Qt::CaseInsensitive))
    {
        s20cText = "S20CFEBE00";
    }
    else
    {
        //请求用户输入CRC在仪表中的存储位置
        bool isOK;
        QString crcAddressQueryData = QInputDialog::getText(NULL,
                                                         "crc address on chip query",
                                                         "Please input crc address on chip, which must\ninclude 6 hexadecimal characters(e.g. FEBE00)\n",
                                                         QLineEdit::Normal,
                                                         "",
                                                         &isOK);
        //校验输入信息
        QRegExp regExp("^[a-fA-F\\d]{6}$");

        if(isOK && regExp.exactMatch(crcAddressQueryData))
        {
            s20cText = "S20C" + crcAddressQueryData.toUpper();
        }
        else
        {
            QMessageBox::warning(this, "Warnning", "invalid address on chip", QMessageBox::Yes);
            return;
        }
    }

    QMessageBox::information(this, "Tips",
                             "part number: " + partNumber + ", sw ver: " + softwareVersion + ", crc address on chip: 0x" + s20cText.right(6),
                             QMessageBox::Yes);
    qDebug() << "crc address: " << s20cText;

    for(int cnt = 0; cnt < 4; ++cnt)
        s20cText.append(QString::number(crc, 16));

    if(chkSum <= 0x0f)
        s20cText += "0"+ QString::number(chkSum, 16);
    else
        s20cText += QString::number(chkSum, 16);

    s20cText = s20cText.toUpper();
    m_leDiagnosisS20C->setText(s20cText);
    //将S20C数据添加换行符并写入排序完成的文件的倒数第二行
    s20cText.append('\n');
    originalS19FileStringList.insert(originalS19FileStringList.size() - 1, s20cText);

    //生成固件并在文件夹中定位此文件
    QString diagnosisFileName = fileInfo.at(FILE_NAME);
    QString timeInfo = QDateTime::currentDateTime().toString("yyyy-MM-dd_HH-mm-ss");
    diagnosisFileName = diagnosisFileName.left(diagnosisFileName.size() - 4);
    diagnosisFileName += "_diagnosis_APPLICATION_FILE_" + timeInfo + ".S19";

    QString folderName = "/generatedFirmwaresForDiagnosis/";
    QString dirPath = fileInfo.at(ABSOLUTE_PATH) + folderName;
    QDir dir(dirPath);
    if(!dir.exists())
        dir.mkdir(dirPath);

    QString diagnosisFilePathName = dirPath + diagnosisFileName;
    QFile newFile(diagnosisFilePathName);
    if(!newFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QMessageBox::warning(this, "Warnning", "Cannot open " + diagnosisFilePathName, QMessageBox::Yes);
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
    if(m_leDiagnosisS021->text().contains(DIAG_M1AFL2_PARTNUMBER.toLatin1().toHex(), Qt::CaseInsensitive))
    {
        generateFiles(CMD_GEN_M1AFL2_FLASH_DRIVER, dirPath, false);
    }
    else if(m_leDiagnosisS021->text().contains(DIAG_T19_PARTNUMBER.toLatin1().toHex(), Qt::CaseInsensitive))
    {
        generateFiles(CMD_GEN_T19_FLASH_DRIVER, dirPath, false);
    }
    else if(m_leDiagnosisS021->text().contains(DIAG_S51EVFL_PARTNUMBER.toLatin1().toHex(), Qt::CaseInsensitive))
    {
        generateFiles(CMD_GEN_S51EVFL_FLASH_DRIVER, dirPath, false);
    }
    else if(m_leDiagnosisS021->text().contains(DIAG_A13TEV_PARTNUMBER.toLatin1().toHex(), Qt::CaseInsensitive))
    {
        generateFiles(CMD_GEN_A13TEV_FLASH_DRIVER, dirPath, false);
    }
    else
    {
        generateFiles(CMD_GEN_COMMON_FLASH_DRIVER, dirPath, false);
    }

    //windows系统下直接打开该文件夹并选中诊断仪app文件
#ifdef WIN32
    QProcess process;
    QString openNewFileName = diagnosisFilePathName;

    openNewFileName.replace("/", "\\");    //这句windows下必要
    process.startDetached("explorer /select," + openNewFileName);
#endif
}

//将.S19原文件里面的S224代码段，按内存分页升序顺序重新排列（F1、F2、···、FF）
bool MainWindow::sortS19Code(QStringList &originalStringList)
{
    QStringList stringListS0AndS9;
    QStringList stringListS1;
    QStringList stringListS2[16]; //F0-FF最多16组

    //S0、S9行
    stringListS0AndS9.push_back(originalStringList.first());
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

//将以字符存储的十六进制数字转换为对应数字
int MainWindow::hexCharToHex(char src)
{
    int ret = -1;

    if(src >= '0' && src <= '9')
    {
        ret = src - '0';
    }
    else if(src >= 'A' && src <= 'F')
    {
        ret = src - 'A' + 10;
    }
    else if(src >= 'a' && src <= 'f')
    {
        ret = src - 'a' + 10;
    }

    return ret;
}

//计算CRC，参考《ECU bootloader and programming implementation specification》
unsigned short MainWindow::calcCRC16(QList<unsigned char> data_list)
{
    unsigned short crc = 0xffff; /* initial value */
    unsigned char tmp = 0;
    int cnt = 0;

    for(cnt = 0; cnt < data_list.size(); ++cnt)
    {
        tmp = (crc >> 8) ^ data_list.at(cnt);
        crc = (crc << 8) ^ crcLookupTable[tmp];
    }

    return crc;
}

//计算S20C行的checksum
unsigned char MainWindow::calcChecksum(unsigned short crc)
{
    unsigned char checkSum = 0;
    unsigned char crcLow = 0;
    unsigned char crcHigh = 0;
    unsigned short temp = 0;

    crcLow = crc & 0xff;
    crcHigh = (crc >> 8) & 0xff;
    //S2 0C FE8000 XX XX  XX XX  XX XX  XX XX  CHK
    temp = 0x0C + 0xFE + 0x80 + 0x00 + crcLow + crcHigh + crcLow + crcHigh + crcLow + crcHigh + crcLow + crcHigh;
    checkSum = temp & 0xff;
    checkSum = 0xff - checkSum;
    return checkSum;
}

//根据seed计算得出key值
unsigned short MainWindow::calculateKey(unsigned short seed)
{
    enum
    {
        TOPBIT = 0x8000,
        POLYNOM_1 = 0x8408,
        POLYNOM_2 = 0x8025,
        BITMASK = 0x0080,
        INITIAL_REMINDER = 0xFFFE,
        MSG_LEN = 2
    };
    unsigned char bSeed[2] = { 0 };
    unsigned char n = 0;
    unsigned char i = 0;
    unsigned short remainder = INITIAL_REMINDER;

    bSeed[0] = (unsigned char)(seed >> 8); /* MSB */
    bSeed[1] = (unsigned char)seed; /* LSB */
    for (n = 0; n < MSG_LEN; n++)
    {
        /* Bring the next byte into the remainder. */
        remainder ^= ((bSeed[n]) << 8);

        /* Perform modulo-2 division, a bit at a time. */
        for (i = 0; i < 8; i++)
        {
            /* Try to divide the current data bit. */
            if (remainder & TOPBIT)
            {
                if(remainder & BITMASK)
                {
                    remainder = (remainder << 1) ^ POLYNOM_1;
                }
                else
                {
                    remainder = (remainder << 1) ^ POLYNOM_2;
                }
            }
            else
            {
                remainder = (remainder << 1);
            }
        }
    }

    return remainder;
}

//根据seed计算得出key值
void MainWindow::dealWithCalculateKeyCommand(void)
{
    showHelpInfo(CMD_HELP_DIAG_CALCULATE_KEY);

    //请求用户输入解锁种子
    bool isOK;
    QString seedQueryData = QInputDialog::getText(NULL,
                                                  "seed query",
                                                  "Please input seed, up to 4 characters\nin addition to whitespace\n",
                                                  QLineEdit::Normal,
                                                  "",
                                                  &isOK);

    //删除输入的空格等空白符
    seedQueryData.remove(QRegExp("\\s"));
    qDebug() << seedQueryData;

    //校验输入信息
    QRegExp regExp("^[0-9a-fA-F]{4}$");

    if(isOK && regExp.exactMatch(seedQueryData))
    {
        unsigned short seed = 0;
        unsigned short key = 0;

        bool resultOK = false;
        seed = seedQueryData.toUShort(&resultOK, 16);

        if(!resultOK)
        {
            QMessageBox::warning(this, "Warnning", "fialed to calculate seed", QMessageBox::Yes);
        }
        else
        {
            key = calculateKey(seed);
            QString keyHexStr = QString::number(key, 16).toUpper();

            if(keyHexStr.size() > 4)
            {
                QMessageBox::warning(this, "Warnning", "length of key should not be greater than 4", QMessageBox::Yes);
                return;
            }

            //若结果的十六进制字符数小于4，则在前面补0，最终要补齐至4个字符
            for(int cnt = 0; cnt < 4 - keyHexStr.size(); ++cnt)
                keyHexStr.prepend('0');

            qDebug() << keyHexStr;

            QClipboard *clipboard = QApplication::clipboard();
            clipboard->clear();
            clipboard->setText("2704" + keyHexStr);

            ptOutputWnd->clear();
            ptOutputWnd->appendPlainText("*************************************");
            ptOutputWnd->appendPlainText("   种子: [" + seedQueryData.toUpper() + "]  ->  密钥: [" + keyHexStr + "]");
            ptOutputWnd->appendPlainText("*************************************");
            ptOutputWnd->appendPlainText("结果已经拷贝至系统剪贴板中:[2704" + keyHexStr + "]");
            ptOutputWnd->appendPlainText("*************************************");

            QMessageBox::information(this, "Tips",
                                     "the key in hexadecimal is 【" + keyHexStr + "】, and the result has already been copied to os clipboard",
                                     QMessageBox::Yes);
        }
    }
    else
    {
        QMessageBox::warning(this, "Warnning", "invalid seed", QMessageBox::Yes);
    }
}

//帮助信息
void MainWindow::showHelpInfo(CmdType cmd)
{
    QStringList hlpInfo;

    if(CMD_HELP == cmd)
    {
        hlpInfo << tr("《命令列表》");
        hlpInfo << tr("0 欢迎使用本软件，本页面介绍软件支持的指令，及其功能定义.");
        hlpInfo << tr("0.1 将<u>switch function</u>（或<u>input command</u>）切换至<u>run command</u>，"
                             "在命令输入框键入<u>:help</u>或<u>:hlp</u>或<u>:?</u>后按回车键进入本页面.");
        hlpInfo << tr("0.2 其它命令的使用方法同0.1节，<u>指令输入不区分大小写</u>.");

        hlpInfo << tr("1 软件自身功能.");
        hlpInfo << tr("1.1 清屏.");
        hlpInfo << tr("1.1.1 定义：清空本软件屏幕当前正在显示的内容.");
        hlpInfo << tr("1.1.2 指令：<u>:clear screen</u>或<u>:cs</u>.");
        hlpInfo << tr("1.2 全屏显示.");
        hlpInfo << tr("1.2.1 定义：将软件切换至全屏显示.");
        hlpInfo << tr("1.2.2 指令：<u>:full screen</u>或<u>:fs</u>.");
        hlpInfo << tr("1.3 正常显示.");
        hlpInfo << tr("1.3.1 定义：将软件切换至正常大小显示.");
        hlpInfo << tr("1.3.2 指令：<u>:normal screen</u>或<u>:ns</u>.");
        hlpInfo << tr("1.4 保存配置.");
        hlpInfo << tr("1.4.1 定义：保存本软件的配置项，将在软件目录下生成config.json文件.");
        hlpInfo << tr("1.4.2 指令：<u>:save config file</u>或<u>:scf</u>.");
        hlpInfo << tr("1.4.3 备注：当前仅实现接口，尚未有实际需要存储的配置项.");
        hlpInfo << tr("1.5 加载配置.");
        hlpInfo << tr("1.5.1 定义：加载本软件的配置项，将加载软件目录下的config.json文件.");
        hlpInfo << tr("1.5.2 指令：<u>:load config file</u>或<u>:lcf</u>.");
        hlpInfo << tr("1.5.3 备注：当前仅实现接口，尚未有实际需要加载的配置项.");
        hlpInfo << tr("1.6 退出程序.");
        hlpInfo << tr("1.6.1 定义：退出本软件.");
        hlpInfo << tr("1.6.2 指令：<u>:quit</u>或<u>:q</u>.");

        hlpInfo << tr("2 Windows系统工具快捷开启功能.");
        hlpInfo << tr("2.1 运行Windows系统指令通运方法.");
        hlpInfo << tr("2.1.1 定义：像Windows系统自带的“运行”软件一样运行Windows系统指令.");
        hlpInfo << tr("2.1.2 指令：<u>::command</u>，其中<u>command</u>为Windows系统能识别的指令.");
        hlpInfo << tr("2.1.3 备注：受限于QT的运行机制，部分Windows系统指令可能不会被正确执行.");
        hlpInfo << tr("2.2 计算器.");
        hlpInfo << tr("2.2.1 定义：快速开启Windows系统自带的“计算器”软件.");
        hlpInfo << tr("2.2.2 指令：<u>:calculator</u>或<u>:calc</u>或<u>:c</u>.");
        hlpInfo << tr("2.3 任务管理器.");
        hlpInfo << tr("2.3.1 定义：快速开启Windows系统自带的“任务管理器”软件.");
        hlpInfo << tr("2.3.2 指令：<u>:taskmgr</u>或<u>:tm</u>.");
        hlpInfo << tr("2.4 画图.");
        hlpInfo << tr("2.4.1 定义：快速开启Windows系统自带的“画图”软件.");
        hlpInfo << tr("2.4.2 指令：<u>:mspaint</u>或<u>:mp</u>.");

        hlpInfo << tr("3 天有为开发工具功能.");
        hlpInfo << tr("3.1 bootloader合成.");
        hlpInfo << tr("3.1.1 定义：奇瑞M1系列bootloader合成方法介绍.");
        hlpInfo << tr("3.1.2 指令：<u>:bootloader?</u>或<u>:b?</u>.");
        hlpInfo << tr("3.2 诊断仪app合成.");
        hlpInfo << tr("3.2.1 定义：奇瑞M1系列诊断仪app合成方法介绍.");
        hlpInfo << tr("3.2.2 指令：<u>:diagnosis?</u>或<u>:d?</u>.");
        hlpInfo << tr("3.3 诊断仪app合成辅助工具.");
        hlpInfo << tr("3.3.1 生成flash driver文件.");
        hlpInfo << tr("3.3.1.1.1 定义：独立生成一个m1afl2诊断仪用flash driver文件.");
        hlpInfo << tr("3.3.1.1.2 指令：<u>:m1afl2 flash driver</u>或<u>:mfd</u>.");
        hlpInfo << tr("3.3.1.2.1 定义：独立生成一个t19诊断仪用flash driver文件.");
        hlpInfo << tr("3.3.1.2.2 指令：<u>:t19 flash driver</u>或<u>:tfd</u>.");
        hlpInfo << tr("3.3.1.3.1 定义：独立生成一个s51evfl诊断仪用flash driver文件.");
        hlpInfo << tr("3.3.1.3.2 指令：<u>:s51evfl flash driver</u>或<u>:sfd</u>.");
        hlpInfo << tr("3.3.1.4.1 定义：独立生成一个通用奇瑞诊断仪用flash driver文件.");
        hlpInfo << tr("3.3.1.4.2 指令：<u>:common flash driver</u>或<u>:cfd</u>或<u>:fd</u>.");
        hlpInfo << tr("3.3.1.4 备注：用3.2节方法也会自动生成flash driver文件.");
        hlpInfo << tr("3.4 生成erase eeprom firmware文件.");
        hlpInfo << tr("3.4.1 定义：生成用于清除仪表eeprom的.S19固件.");
        hlpInfo << tr("3.4.2 指令：<u>:erase eeprom</u>或<u>:ee</u>.");
        hlpInfo << tr("3.4.3 备注：烧录此固件后会擦除仪表原来的app固件，需要重新烧录app固件.");
        hlpInfo << tr("3.5 生成M1系列boot code文件.");
        hlpInfo << tr("3.5.1 定义：生成M1系列boot代码段的.S19固件.");
        hlpInfo << tr("3.5.2 指令：<u>:m boot code</u>或<u>:mbc</u>.");
        hlpInfo << tr("3.5.3 备注：此固件不可单独烧录至仪表，需要合成到仪表app固件中，合成方法参考3.1节.");
        hlpInfo << tr("3.6 生成T1系列boot code文件.");
        hlpInfo << tr("3.6.1 定义：生成T1系列boot代码段的.S19固件.");
        hlpInfo << tr("3.6.2 指令：<u>:t boot code</u>或<u>:tbc</u>.");
        hlpInfo << tr("3.6.3 备注：此固件不可单独烧录至仪表，需要合成到仪表app固件中，合成方法参考3.1节.");
        hlpInfo << tr("3.7 生成S51EVFL的boot code文件.");
        hlpInfo << tr("3.7.1 定义：生成S51EVFL的boot代码段的.S19固件.");
        hlpInfo << tr("3.7.2 指令：<u>:s boot code</u>或<u>:sbc</u>.");
        hlpInfo << tr("3.7.3 备注：此固件不可单独烧录至仪表，需要合成到仪表app固件中，合成方法参考3.1节.");
        hlpInfo << tr("4 开发辅助工具.");
        hlpInfo << tr("4.1 文件转字符串工具.");
        hlpInfo << tr("4.1.1 定义：将指定文件的所有字节生成C/C++语言能识别的数组，并存储为.h文件.");
        hlpInfo << tr("4.1.2 指令：<u>:convert code to string</u>或<u>:c2s</u>.");
        hlpInfo << tr("4.2 根据种子计算密钥.");
        hlpInfo << tr("4.2.1 定义：某些诊断服务需要解锁，本功能根据输入的种子来计算对应的密钥.");
        hlpInfo << tr("4.2.2 指令：<u>:calculate key</u>或<u>:calc key</u>或<u>:ck</u>.");
        hlpInfo << tr("4.2.3 备注：输入<u>:calculate key?</u>或<u>:calc key?</u>或<u>:ck?</u>，显示安全解锁操作步骤帮助信息.");
        hlpInfo << tr("4.3 由Img2Lcd生成的位图C数组数据压缩工具.");
        hlpInfo << tr("4.3.1 定义：由Img2Lcd生成的位图C数组数据，将其中的全部0x00或0xff，按照出现的次数进行压缩.");
        hlpInfo << tr("4.3.2 指令：<u>:compress bmp</u>或<u>:cb</u>.");
    }
    else if(CMD_HELP_BOOTLOADER == cmd)
    {
        hlpInfo << tr("《BootLoader合成工具使用方法》");
        hlpInfo << tr("0 将<u>switch function</u>（或<u>input command</u>）切换至<u>add bootloader to firmware</u>.");
        hlpInfo << tr("1 加载bootloader代码段的两种方式.");
        hlpInfo << tr("1.1 方式一：加载默认bootloader代码段，先勾选<u>use default</u>，再更改其左侧的下拉选择框来选择加载特定型号的默认bootloader代码段.");
        hlpInfo << tr("1.2 方式二：加载其它bootloader代码段，去勾选<u>default</u>，点击<u>load bootloader</u>按钮选择其它bootloader文件.");
        hlpInfo << tr("2 点击<u>load file</u>按钮选择.S19原app文件.");
        hlpInfo << tr("3 点击<u>generate</u>按钮生成含bootloader的新app文件,并自动打开该文件所在的目录且选中该文件.");
    }
    else if(CMD_HELP_DIAGNOSIS == cmd)
    {
        hlpInfo << tr("《诊断仪app生成工具使用方法》");
        hlpInfo << tr("0 将<u>switch function</u>（或<u>input command</u>）切换至<u>gen firmware for diagnosis</u>.");
        hlpInfo << tr("1 点击<u>load file</u>按钮选择.S19原app文件.");
        hlpInfo << tr("2 输入S021数据，该数据最终将位于app的第一行.");
        hlpInfo << tr("2.1 在S021输入框输入<u>:t</u>并按回车键获取T19预置的S021数据；输入<u>:m</u>并按回车键获取m1afl2预置的S021数据；"
                      "输入<u>:s</u>并按回车键获取s51evfl预置的S021数据；输入<u>:a</u>并按回车键获取a13tev预置的S021数据.");
        hlpInfo << tr("2.2 正确输入S021数据后，将光标置于S021数据所在的输入框后，点击回车键可以修改版本号，版本号格式需严格匹配<u>xx.xx.xx</u>,x为0-9或a-f,字母不区分大小写，最终按大写字母写入文件.");
        hlpInfo << tr("2.3 将光标置于S021数据所在的输入框后，当S021输入框为空时按下回车键，会请求输入零件号，之后会请求输入软件版本信息，然后自动合成S021行数据，数据生成后依然可以使用2.1节的方法修改版本号.");
        hlpInfo << tr("3 点击<u>generate</u>按钮，生成诊断仪app文件,并自动打开该文件所在的目录且选中该文件.");
        hlpInfo << tr("4 该文件夹下还将自动生成flash driver文件，请将诊断仪app文件和flash driver文件一同加入压缩包提供给使用者.");
    }
    else if(CMD_HELP_DIAG_CALCULATE_KEY == cmd)
    {
        hlpInfo << tr("《安全解锁操作步骤》");
        hlpInfo << tr("0 在busmaster软件诊断窗口将[Send Tester Present]勾选为[ON]，激活3E服务.");
        hlpInfo << tr("1 在busmaster软件诊断窗口发送[10 03]，切换到扩展会话.");
        hlpInfo << tr("2 在busmaster软件诊断窗口发送[27 03]，请求计算安全密钥的种子.");
        hlpInfo << tr("3 将busmaster软件诊断窗口收到的[67 03 xx xx]中的后两个字节即[xx xx]输入到弹出的[seed query]窗口中.");
        hlpInfo << tr("4 点击[seed query]窗口的[OK]将会返回计算好的安全密钥.");
        hlpInfo << tr("5 输入密钥.");
        hlpInfo << tr("5.1 方法一：在busmaster软件诊断窗口发送[27 04 hh hh]，其中[hh hh]为第4步所得的数值.");
        hlpInfo << tr("5.2 方法二：第4步完成后，程序已经将结果复制到系统剪贴板中，在busmaster软件诊断发送窗口，使用鼠标右键单击并选择粘贴，然后发送即可.");
    }

    ptOutputWnd->clear();
    foreach(QString elem, hlpInfo)
    {
        //另起一个新段落
        if(elem.startsWith("0 ") || elem.startsWith("1 ") || elem.startsWith("2 ") || elem.startsWith("3 ") || elem.startsWith("4 ") || elem.startsWith("5 "))
        {
            ptOutputWnd->appendPlainText(QString());
        }

        ptOutputWnd->appendHtml(elem);
    }

    //光标移动到最顶部，否则将显示文件末尾
    QTextCursor cursor = ptOutputWnd->textCursor();
    cursor.movePosition(QTextCursor::Start);
    ptOutputWnd->setTextCursor(cursor);
}

//处理配置文件
void MainWindow::procConfigFile(CmdType cmd)
{
    if(CMD_SAVE_CONFIG_FILE == cmd)
    {
        QString fileName = QDir::currentPath() + '/' + CONFIG_FILE_NAME;
        QFile file(fileName);
        if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            QMessageBox::warning(this, "Warnning", "Cannot open " + fileName, QMessageBox::Yes);
            return;
        }

        QJsonObject jsonConfig;
        jsonConfig.insert("version", SOFTWARE_VERSION);
        jsonConfig.insert("autoupdate", true);

        QJsonDocument document;
        document.setObject(jsonConfig);
        QByteArray byteArray = document.toJson(QJsonDocument::Indented);
        QString jsonEncodedString(byteArray);

        QTextStream out(&file);
        out << jsonEncodedString;
        file.close();
    }
    else if(CMD_LOAD_CONFIG_FILE == cmd)
    {
        QString fileName = QDir::currentPath() + '/' + CONFIG_FILE_NAME;
        QFile file(fileName);
        if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
//            QMessageBox::warning(this, "Warnning", "Cannot find " + fileName, QMessageBox::Yes);
            return;
        }

        QTextStream in(&file);
        QString jsonEncodedString = in.readAll();
        file.close();

        QJsonParseError jsonError;
        QJsonDocument parseDoucment = QJsonDocument::fromJson(jsonEncodedString.toLocal8Bit(), &jsonError);
        if(QJsonParseError::NoError == jsonError.error)
        {
            if(parseDoucment.isObject())
            {
                QJsonObject docObj = parseDoucment.object();
                if(docObj.contains("autoupdate"))
                {
                    QJsonValue value = docObj.take("autoupdate");
                    if(value.isBool())
                    {
                        if(value.toBool())
                        {
                            autoUpdateTypeB();

                            if(docObj.contains("version"))
                            {
                                QJsonValue value = docObj.take("version");
                                if(value.isString())
                                {
                                    //autoUpdate(value.toString());
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

void MainWindow::autoUpdateTypeB()
{
    versionFilePathName = QCoreApplication::applicationDirPath() + versionFilePathName;

    QFile tmpFile(versionFilePathName);
    if(tmpFile.exists())
    {
        tmpFile.remove();
    }

    qDebug() << "start download version.txt";
//    ptOutputWnd->appendPlainText("start download version.txt");

    QString downloadVersionCmd = "certutil.exe -urlcache -split -f " + VERSION_DOWNLOAD_URL + " " + versionFilePathName;
    QProcess::startDetached(downloadVersionCmd);

    versionDetectTimer = new QTimer;
    connect(versionDetectTimer, SIGNAL(timeout()), this, SLOT(versionDetectTimerTimeout()));
    versionDetectTimer->start(1000);
}

void MainWindow::versionDetectTimerTimeout()
{
    static int howmany1sPassed = 0;
    static bool isDownloadSuccess = false;
    ++howmany1sPassed;

    qDebug() << "howmany1sPassed(version): " << howmany1sPassed;
//    ptOutputWnd->appendPlainText("howmany1sPassed(version): " + QString::number(howmany1sPassed));

    if(120 == howmany1sPassed && !isDownloadSuccess)
    {
        versionDetectTimer->stop();
        return;
    }

    QFile versionFile(versionFilePathName);
    if(versionFile.exists())
    {
        isDownloadSuccess = true;
        versionDetectTimer->stop();

        if(!versionFile.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            QMessageBox::warning(this, "Warnning", "Cannot open " + versionFilePathName, QMessageBox::Yes);
            return;
        }

        QTextStream fileIn(&versionFile);
        QStringList fileStringList;
        while(!fileIn.atEnd())
        {
            QString readStr = fileIn.readLine();

            qDebug() << "version on server: " << readStr;
//            ptOutputWnd->appendPlainText("version on server: " + readStr);

            if(!readStr.isEmpty())
                fileStringList.push_back(readStr);
        }

        versionFile.close();
        versionFile.remove();

        if(fileStringList.at(0) > SOFTWARE_VERSION && fileStringList.at(0).startsWith('v'))
        {
            int ret = QMessageBox::question(this, tr("自动升级"), "检测到新版本" + fileStringList.at(0) + "，当前版本" + SOFTWARE_VERSION + "，是否升级？", QMessageBox::Yes, QMessageBox::No);
            if(QMessageBox::No == ret)
                return;

            appDetectTimer = new QTimer;
            connect(appDetectTimer, SIGNAL(timeout()), this, SLOT(appDetectTimerTimeout()));
            appDetectTimer->start(1000);

            qDebug() << "start download app";
//            ptOutputWnd->appendPlainText("start download app");

            QString fileName = appNameFirst + fileStringList.at(0) + appNameLast;
            appFilePathName = QCoreApplication::applicationDirPath() + '/' + fileName;

            QFile tmpFile(appFilePathName);
            if(tmpFile.exists())
            {
                tmpFile.remove();
            }

            QString downloadAppCmd = "certutil.exe -urlcache -split -f " + APP_DOWNLOAD_URL + fileName + " " + appFilePathName;
            QProcess::startDetached(downloadAppCmd);
        }
    }
}

void MainWindow::appDetectTimerTimeout()
{
    static int howmany1sPassed = 0;
    static bool isDownloadSuccess = false;
    ++howmany1sPassed;

    qDebug() << "howmany1sPassed(app): " << howmany1sPassed;
//    ptOutputWnd->appendPlainText("howmany1sPassed(app): " + QString::number(howmany1sPassed));

    if(300 == howmany1sPassed && !isDownloadSuccess)
    {
        QMessageBox::critical(NULL, tr("自动升级"), tr("更新可能失败了..."));
        appDetectTimer->stop();
        return;
    }

    QFile appFile(appFilePathName);
    if(appFile.exists())
    {
        isDownloadSuccess = true;
        appDetectTimer->stop();

        qDebug() << "app file size: " << appFile.size();
//        ptOutputWnd->appendPlainText("app file size: " + QString::number(appFile.size()));

        if(appFile.size() >= 500 * 1024)
        {
            int ret = QMessageBox::information(this, tr("自动升级"), "更新成功，是否查看版本更新日志？", QMessageBox::Yes, QMessageBox::No);
            if(QMessageBox::Yes == ret)
                QDesktopServices::openUrl(QUrl(QLatin1String("https://github.com/bingshuizhilian/QTPROJECTS-FIRMWARE_GENERATOR/releases")));

            //WINDOWS环境下，选中该文件
#ifdef WIN32
            QProcess process;
            QString openFileName = appFilePathName;

            openFileName.replace("/", "\\");    //***这句windows下必要***
            process.startDetached("explorer /select," + openFileName);
#endif
        }
        else
        {
            appFile.remove();
            QMessageBox::critical(NULL, tr("自动升级"), "更新失败了...");
        }
    }
}

//基于QT网络库的下载虽已实现网络传输，但是不能下载一些需要的链接，待后续有时间再分析
void MainWindow::autoUpdate(QString local_version)
{
    qDebug() << local_version;

    versionFilePathName = QCoreApplication::applicationDirPath() + versionFilePathName;

    QFile tmpFile(versionFilePathName);
    if(tmpFile.exists())
    {
        tmpFile.remove();
    }

    downloadFile = new QFile(versionFilePathName);
    if(!downloadFile->open(QIODevice::WriteOnly))
    {
        qDebug() << "cannot open file";
        return;
    }

    //检测服务器上的软件版本号
    m_networkAccessMngr = new QNetworkAccessManager;
    QUrl url(VERSION_DOWNLOAD_URL);
    QNetworkRequest request(url);
//    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/octet-stream");

    m_httpReply = m_networkAccessMngr->get(request);//发送请求

    connect(m_httpReply, SIGNAL(readyRead()), this, SLOT(httpReadContent()));
    connect(m_networkAccessMngr, SIGNAL(finished(QNetworkReply*)), this, SLOT(httpReplyFinished(QNetworkReply*)));
    connect(m_httpReply, SIGNAL(error(QNetworkReply::NetworkError)),this,SLOT(httpDownloadError(QNetworkReply::NetworkError)));

    //询问是否需要升级
//    int ret = QMessageBox::question(this, tr("自动升级"), tr("需要更新到最新版本程序吗？"), QMessageBox::Yes, QMessageBox::No);
//    if(QMessageBox::No == ret)
//        return;

    //下载服务器上的最新版本软件


}

void MainWindow::httpReadContent()
{
    static bool isConnectToDownloadProgress = 1;
    if(isConnectToDownloadProgress)
    {
        isConnectToDownloadProgress = 0;
        connect(m_httpReply, SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(httpDownloadProgress(qint64, qint64)));
    }

    QByteArray reply = m_httpReply->readAll();
    ptOutputWnd->appendPlainText(reply);
//    qDebug() << "______write file: " <<
    downloadFile->write(reply);
}

void MainWindow::httpReplyFinished(QNetworkReply *reply)
{
    reply->deleteLater();
    downloadFile->flush();
    downloadFile->close();

    if(reply->error() == QNetworkReply::NoError)
    {
        QMessageBox::information(NULL, tr("info"), "Download success!!!");
    }
    else
    {
        QFile tmpFile(versionFilePathName);
        if(tmpFile.exists())
        {
            tmpFile.remove();
        }

        QMessageBox::critical(NULL, tr("Error"), "Download failed!!!");
    }
}

void MainWindow::httpDownloadError(QNetworkReply::NetworkError error)
{
    qDebug() << "httpDownloadError: " << error;
}

void MainWindow::httpDownloadProgress(qint64 bytes_received, qint64 bytes_total)
{
    qDebug() << "httpDownloadProgress: " << bytes_received << bytes_total << QString(" -> %1%").arg(bytes_received * 100 / bytes_total);
}

//将boot code生成为字符串常量，当boot code更新时，调用此函数将其转换为数组
void MainWindow::generateCharArray()
{
    ptOutputWnd->clear();
    ptOutputWnd->appendPlainText("欢迎使用文件转字符串工具");

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

    targetCodeStringList.push_front("const char* DEFAULT_XX_CODE =\n");
    targetCodeStringList.push_front("#define __DEFAULT_XX_CODE_H__\n\n");
    targetCodeStringList.push_front("#ifndef __DEFAULT_XX_CODE_H__\n");
    targetCodeStringList.push_back("\n#endif\n");

    for(auto& elem:targetCodeStringList)
        qDebug()<<elem<<endl;

    QString tmpFileName = "defaultXxCode_";
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

void MainWindow::compressCArrayOfBitmap()
{
    ptOutputWnd->clear();
    ptOutputWnd->appendPlainText("欢迎使用Image2Lcd生成的C数组的数组压缩工具");

    const int DEVIDE_AMOUNT = 255; //下位机存储宽和高各用一个字节，此处应确保此值不大于255（即0xff）
    QString filePathName = QFileDialog::getOpenFileName();
    QString filePath = QFileInfo(filePathName).absolutePath();

    qDebug()<<filePathName<<endl<<filePath<<endl;

    if(filePathName.isEmpty())
    {
        QMessageBox::warning(this, "Warnning", "compress failed, please select a file", QMessageBox::Yes);
        return;
    }

    QFile targetFile(filePathName);
    if(!targetFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::warning(this, "Warnning", "Cannot open " + filePathName, QMessageBox::Yes);
        return;
    }

    this->resize(640, 460);
    ptOutputWnd->clear();

    QTextStream targetFileIn(&targetFile);
    QStringList originalCodeStringList;
    while(!targetFileIn.atEnd())
    {
        QString readStr = targetFileIn.readLine();

        if(!readStr.isEmpty())
            originalCodeStringList.push_back(readStr);

//        ptOutputWnd->appendPlainText(readStr); //debug
    }

    targetFile.close();

    QString tmpStr = originalCodeStringList.first();

    if(!tmpStr.startsWith("const unsigned char gImage_", Qt::CaseInsensitive) || !tmpStr.endsWith("*/", Qt::CaseInsensitive))
    {
        QMessageBox::warning(this, "Warnning", "make sure the C source file was generated by Image2Lcd", QMessageBox::Yes);
        return;
    }

    //暂未考虑bmp宽或高大于256的情况，因为3.5" TFT实际使用到的图片尺寸在任意方向上不会且不能超过248
    QString widthAndHeight = tmpStr.mid(tmpStr.lastIndexOf(',') - 14, 4);
    widthAndHeight += ", ";
    widthAndHeight += tmpStr.mid(tmpStr.lastIndexOf(',') - 4, 4);
    widthAndHeight += ", //width: " + QString::number(tmpStr.mid(tmpStr.lastIndexOf(',') - 12, 2).toInt(nullptr, 16));
    widthAndHeight += ", height: " + QString::number(tmpStr.mid(tmpStr.lastIndexOf(',') - 2, 2).toInt(nullptr, 16));

    if(tmpStr.mid(tmpStr.lastIndexOf(',') - 17, 2).toInt(nullptr, 16) != 0 || tmpStr.mid(tmpStr.lastIndexOf(',') - 7, 2).toInt(nullptr, 16) != 0)
    {
        int realWidth = 0, realHeight = 0;
        realWidth = tmpStr.mid(tmpStr.lastIndexOf(',') - 17, 2).toInt(nullptr, 16) << 8 | tmpStr.mid(tmpStr.lastIndexOf(',') - 12, 2).toInt(nullptr, 16);
        realHeight = tmpStr.mid(tmpStr.lastIndexOf(',') - 7, 2).toInt(nullptr, 16) << 8 | tmpStr.mid(tmpStr.lastIndexOf(',') - 2, 2).toInt(nullptr, 16);
        widthAndHeight += " [!!! Be careful, realWidth: " + QString::number(realWidth) + ", realHeight: " + QString::number(realHeight) + " !!!]";
    }

    QStringList targetFormattedStringList;
    targetFormattedStringList << "static const unsigned char bmp[] =\n{" << widthAndHeight;
    originalCodeStringList.removeFirst();
    originalCodeStringList.last().remove("};");

    QStringList tmpStringList;
    for(auto elem: originalCodeStringList)
        tmpStringList += elem.split(',');

    tmpStringList.removeAll("");
    //在结尾添加一个结束标记，否则当数据全为0x00或0xff且数据量较大时，下面的正则表达式查找第一个不为0x00或0xff的数据非常耗时
    tmpStringList.append("END_FLAG");

    //debug
//    for(auto elem: tmpStringList)
//        ptOutputWnd->appendPlainText(elem);

    QStringList targetStringList;
    for(int index = 0; index < tmpStringList.size(); ++index)
    {
        if(0 == tmpStringList.at(index).compare("0X00", Qt::CaseInsensitive) || 0 == tmpStringList.at(index).compare("0XFF", Qt::CaseInsensitive))
        {
            targetStringList << tmpStringList.at(index);

            if(tmpStringList.size() - 1 == index)
            {
                targetStringList << "1";
            }
            else
            {
                QRegExp regexp;
                if(0 == tmpStringList.at(index).compare("0X00", Qt::CaseInsensitive))
                    regexp.setPattern("^((?!0[xX]00).)*$");
                else
                    regexp.setPattern("^((?!0[xX][fF][fF]).)*$");

                int matchIndex = tmpStringList.indexOf(regexp, index);
                if(matchIndex - index <= DEVIDE_AMOUNT)
                {
                    targetStringList << QString::number(matchIndex - index);
                }
                else
                {
                    for(int cnt = 0; cnt < (matchIndex - index) / DEVIDE_AMOUNT; ++cnt)
                    {
                        targetStringList << QString::number(DEVIDE_AMOUNT);
                        targetStringList << tmpStringList.at(index);
                    }

                    if((matchIndex - index) % DEVIDE_AMOUNT != 0)
                    {
                        targetStringList << QString::number((matchIndex - index) % DEVIDE_AMOUNT);
                    }
                    else
                    {
                        //移除多添加的0x00或0xff
                        targetStringList.removeLast();
                    }
                }

                //for循环马上会另index自增1，提前减去1后接下来才能定位到下一个非0x00或0xff的元素，即matchIndex所指向的元素
                index = matchIndex - 1;
            }
        }
        else
        {
            targetStringList << tmpStringList.at(index);
        }
    }

    //移除设置的"END_FLAG"
    targetStringList.removeLast();

    for(int index = 0; index < targetStringList.size(); ++index)
    {
        int numOfCharInElem = targetStringList.at(index).size();
        for(int cnt = 0; numOfCharInElem < 4 && cnt < 4 - numOfCharInElem; ++cnt)
            targetStringList[index].prepend(' ');

        if(0 == (index + 1) % 16)
            targetStringList[index] += ",";
        else
            targetStringList[index] += ", ";
    }

    //移除最后一个", "或","，(肯定为二者之一，两个都remove即可保证)
    targetStringList.last().remove(", ");
    targetStringList.last().remove(",");

    QString tmpStr2;
    for(int index = 0; index < targetStringList.size(); ++index)
    {
        if(index % 16 == 0)
        {
            if(!tmpStr2.isEmpty())
                targetFormattedStringList << tmpStr2;

            tmpStr2.clear();
        }

        tmpStr2 += targetStringList.at(index);

        if(index == targetStringList.size() - 1)
            targetFormattedStringList << tmpStr2;
    }

    targetFormattedStringList << "}; //total bytes: " + QString::number(targetStringList.size() + 2);

    for(auto iter = targetFormattedStringList.begin() + 1; iter != targetFormattedStringList.end() - 1; ++iter)
    {
        *iter = (*iter).prepend("    ");
        *iter = (*iter).toUpper();
    }

    //宽度和高度用小写字母
    if(!targetFormattedStringList.isEmpty())
        targetFormattedStringList[1] = targetFormattedStringList[1].left(14) + targetFormattedStringList[1].right(targetFormattedStringList[1].size() - 14).toLower();

    for(auto& elem: targetFormattedStringList)
        ptOutputWnd->appendPlainText(elem);

    QClipboard *clipboard = QApplication::clipboard();
    clipboard->clear();
    clipboard->setText(ptOutputWnd->document()->toPlainText());

    ptOutputWnd->appendPlainText("\n\n***上述结果已经拷贝至系统剪贴板中***");
}

//控件初始化
void MainWindow::componentsInitialization(void)
{
    setWindowTitle(tr("CheryCommonFirmwareGenerator"));

    //窗体名称及状态栏设置
    auto labelAuthorInfo = new QLabel;
    labelAuthorInfo->setStatusTip(tr("click to view source code on github"));
    labelAuthorInfo->setOpenExternalLinks(true);
    labelAuthorInfo->setText(QString::fromLocal8Bit("<style> a {text-decoration: none} </style> <a href = https://www.github.com/bingshuizhilian/QTPROJECTS-FIRMWARE_GENERATOR> contact author </a>"));
    labelAuthorInfo->show();
    ui->statusBar->addPermanentWidget(labelAuthorInfo);

    //选择.S19文件按钮
    m_btnChooseFile = new QPushButton(tr("load file"));
    connect(m_btnChooseFile, &m_btnChooseFile->clicked, this, &selectFilePressed);
    m_btnChooseFile->setStatusTip(tr("select the original .S19 file"));
    //显示.S19文件名
    m_leFileInfo = new QLineEdit;
    m_leFileInfo->setReadOnly(true);

    //选择bootloader文件按钮
    m_btnLoadBootloader = new QPushButton(tr("load bootloader"));
    connect(m_btnLoadBootloader, &m_btnLoadBootloader->clicked, this, &loadBootloaderPressed);
    //显示bootloader文件名，或提示正在使用默认boot code
    m_leBootloaderInfo = new QLineEdit;
    m_leBootloaderInfo->setReadOnly(true);

    //机型选择
    m_cmbPlatformSwitch = new QComboBox;
    connect(m_cmbPlatformSwitch, static_cast<void (QComboBox::*)(const QString&)>(&QComboBox::currentTextChanged), this, &switchPlatformPressed);
    m_cmbPlatformSwitch->setFixedWidth(77);
    m_cmbPlatformSwitch->setInsertPolicy(QComboBox::NoInsert);
    m_cmbPlatformSwitch->setStatusTip("select a platform");
    m_cmbPlatformSwitch->addItem(PLATFORM_STRING_LIST.at(M1AFL2), M1AFL2);
    m_cmbPlatformSwitch->addItem(PLATFORM_STRING_LIST.at(T18), T18);
    m_cmbPlatformSwitch->addItem(PLATFORM_STRING_LIST.at(T19), T19);
    m_cmbPlatformSwitch->addItem(PLATFORM_STRING_LIST.at(S51EVFL), S51EVFL);
    m_cmbPlatformSwitch->addItem(PLATFORM_STRING_LIST.at(A13TEV), A13TEV);

    //是否使用默认boot code选项
    m_ckbUseDefaultBootloader = new QCheckBox(tr("use default"));
    connect(m_ckbUseDefaultBootloader, &m_ckbUseDefaultBootloader->stateChanged, this, &useDefaultBootloaderPressed);
    m_ckbUseDefaultBootloader->setStatusTip(tr("use default boot code when checked"));
    m_ckbUseDefaultBootloader->setChecked(true);

    //生成按钮
    m_btnGenerate = new QPushButton(tr("generate"));
    connect(m_btnGenerate, &m_btnGenerate->clicked, this, &generateButtonPressed);

    //合成诊断仪firmware时需要的额外信息
    m_leDiagnosisS021 = new QLineEdit;
    connect(m_leDiagnosisS021, &m_leDiagnosisS021->returnPressed, this, &s021ReturnPressed);
    m_leDiagnosisS021->setStatusTip("press enter to modify version");
    m_leDiagnosisS20C = new QLineEdit;
    m_leDiagnosisS20C->setReadOnly(true);
    m_leDiagnosisS20C->setStatusTip("crc and checksum infos");

    //命令行输入输出窗口
    m_leRunCommand = new QLineEdit;
    connect(m_leRunCommand, &m_leRunCommand->returnPressed, this, &runCmdReturnPressed);
    m_leRunCommand->setStatusTip("press enter to run command");
    ptOutputWnd = new QPlainTextEdit;
    ptOutputWnd->setReadOnly(true);
    ptOutputWnd->setStatusTip(tr("execute result echo window"));

    //布局控件
    m_gbBootloader = new QGroupBox;
    m_gbBootloader->setTitle(tr("bootloader settings"));
    m_gbS19Selector = new QGroupBox;
    m_gbS19Selector->setTitle(tr("select .S19 file"));
    m_gbDiagnosis = new QGroupBox;
    m_gbDiagnosis->setTitle(tr("diagnosis data settings"));
    m_gbSwitchFunction = new QGroupBox;

    //功能选择下拉框
    m_cmbFunctionSwitch = new QComboBox;
    connect(m_cmbFunctionSwitch, static_cast<void (QComboBox::*)(const QString&)>(&QComboBox::currentTextChanged), this, &switchFunctionPressed);
    m_cmbFunctionSwitch->setInsertPolicy(QComboBox::NoInsert);
    m_cmbFunctionSwitch->setStatusTip("select a function");
    m_cmbFunctionSwitch->addItem(FUNCTION_STRING_LIST.at(BOOTLOADER), FUNCTION_STRING_LIST.at(BOOTLOADER));
    m_cmbFunctionSwitch->addItem(FUNCTION_STRING_LIST.at(DIAGNOSIS), FUNCTION_STRING_LIST.at(DIAGNOSIS));
    m_cmbFunctionSwitch->addItem(FUNCTION_STRING_LIST.at(CMD_HANDLER), FUNCTION_STRING_LIST.at(CMD_HANDLER));
}

//布局初始化
void MainWindow::layoutsInitialization()
{
    //bootloader控件区
    auto layoutBootloader = new QGridLayout;
    auto labelLeBootloader = new QLabel(tr("bootloader"));
    layoutBootloader->addWidget(labelLeBootloader, 0, 0, 1, 1);
    layoutBootloader->addWidget(m_leBootloaderInfo, 0, 1, 1, 2);
    layoutBootloader->addWidget(m_cmbPlatformSwitch, 1, 0, 1, 1);
    layoutBootloader->addWidget(m_ckbUseDefaultBootloader, 1, 1, 1, 1);
    layoutBootloader->addWidget(m_btnLoadBootloader, 1, 2, 1, 1);
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

    //功能选择控件区
    auto swFuncLayout = new QHBoxLayout;
    swFuncLayout->addWidget(m_cmbFunctionSwitch);
    swFuncLayout->addWidget(m_leRunCommand);
    m_gbSwitchFunction->setLayout(swFuncLayout);

    //底部控件区
    auto bottomLayout = new QHBoxLayout;
    bottomLayout->addWidget(m_gbSwitchFunction);
    bottomLayout->addWidget(m_btnGenerate);

    //globle layout
    m_layoutGlobal = new QVBoxLayout;
    m_layoutGlobal->addWidget(ptOutputWnd);
    m_layoutGlobal->addWidget(m_gbBootloader);
    m_layoutGlobal->addWidget(m_gbS19Selector);
    m_layoutGlobal->addWidget(m_gbDiagnosis);
    m_layoutGlobal->addLayout(bottomLayout);

    //设置窗口和其它组件大小
    m_btnGenerate->setFixedHeight(50);
    ui->centralWidget->setLayout(m_layoutGlobal);
}

//命令初始化
void MainWindow::commandsInitialization()
{
    cmdList.push_back({ CMD_HELP, {":help", ":hlp", ":?"} });
    cmdList.push_back({ CMD_HELP_BOOTLOADER, {":bootloader?", ":b?"} });
    cmdList.push_back({ CMD_HELP_DIAGNOSIS, {":diagnosis?", ":d?"} });
    cmdList.push_back({ CMD_HELP_DIAG_CALCULATE_KEY, {":calculate key?", ":calc key?", ":ck?"} });
    cmdList.push_back({ CMD_CLEAR_SCREEN, {":clear screen", ":cs"} });
    cmdList.push_back({ CMD_FULL_SCREEN, {":full screen", ":fs"} });
    cmdList.push_back({ CMD_NORMAL_SCREEN, {":normal screen", ":ns"} });
    cmdList.push_back({ CMD_QUIT_APPLICATION, {":quit", ":q"} });
    cmdList.push_back({ CMD_SAVE_CONFIG_FILE, {":save config file", ":scf"} });
    cmdList.push_back({ CMD_LOAD_CONFIG_FILE, {":load config file", ":lcf"} });
    cmdList.push_back({ CMD_CODE_TO_STRING, {":convert code to string", ":c2s"} });
    cmdList.push_back({ CMD_COMPRESS_BMP, {":compress bmp", ":cb"} });
    cmdList.push_back({ CMD_GEN_M1AFL2_FLASH_DRIVER, {":m1afl2 flash driver", ":mfd"} });
    cmdList.push_back({ CMD_GEN_T19_FLASH_DRIVER, {":t19 flash driver", ":tfd"} });
    cmdList.push_back({ CMD_GEN_S51EVFL_FLASH_DRIVER, {":s51evfl flash driver", ":sfd"} });
    cmdList.push_back({ CMD_GEN_A13TEV_FLASH_DRIVER, {":a13tev flash driver", ":afd"} });
    cmdList.push_back({ CMD_GEN_COMMON_FLASH_DRIVER, {":common flash driver", ":cfd", ":fd"} });
    cmdList.push_back({ CMD_GEN_ERASE_EEPROM, {":erase eeprom", ":ee"} });
    cmdList.push_back({ CMD_GEN_M1_BOOT_CODE, {":m boot code", ":mbc"} });
    cmdList.push_back({ CMD_GEN_T1_BOOT_CODE, {":t boot code", ":tbc"} });
    cmdList.push_back({ CMD_GEN_S51EVFL_BOOT_CODE, {":s boot code", ":sbc"} });
    cmdList.push_back({ CMD_GEN_A13TEV_BOOT_CODE, {":a boot code", ":abc"} });
    cmdList.push_back({ CMD_DIAG_CALCULATE_KEY, {":calculate key", ":calc key", ":ck"} });
#if WIN32
    cmdList.push_back({ CMD_WINDOWS_COMMON, {"::"} });
    //此处要把windows能识别的命令放在stringlist的首位
    cmdList.push_back({ CMD_WINDOWS_CALCULATOR, {":calc", ":calculator", ":c"} });
    cmdList.push_back({ CMD_WINDOWS_TASKMANAGER, {":taskmgr", ":tm"} });
    cmdList.push_back({ CMD_WINDOWS_PAINT, {":mspaint", ":mp"} });
#endif
}
