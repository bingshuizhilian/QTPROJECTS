#include "firmwaregenerator.h"
#include "ui_firmwaregenerator.h"
#include "modeldata.h"
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

FirmwareGenerator::FirmwareGenerator(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //��ʼ���ؼ�
    componentsInitialization();
    //���ò���
    layoutsInitialization();
    //�����г�ʼ��
    commandsInitialization();

    //�������
    procConfigFile(CMD_LOAD_CONFIG_FILE);
}

FirmwareGenerator::~FirmwareGenerator()
{
    delete ui;
}

//�л����ܺ󣬿ؼ��������ϵ�Ĵ���
void FirmwareGenerator::switchFunctionPressed()
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

//�������������а��س�ִ�������¼�
void FirmwareGenerator::runCmdReturnPressed()
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
    case CMD_GEN_FLASH_DRIVER:
    {
        //�����û����������
        bool isOK;
        QString partnumberQueryData = QInputDialog::getText(this,
                                                            QString::fromLocal8Bit("���������"),
                                                            QString::fromLocal8Bit("�����������, ����������16���ַ�"),
                                                            QLineEdit::Normal,
                                                            "",
                                                            &isOK);
        //У��������Ϣ
        QRegExp regExp("^[\\w\\-]{0,16}$");

        if(isOK && regExp.exactMatch(partnumberQueryData))
        {
            QString dirPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
            dirPath.append("/cheryCustomizedFlashDriver/");
            ptOutputWnd->clear();
            ptOutputWnd->appendPlainText(dirPath.left(dirPath.size() - 1));
            generateFiles(findCmd, dirPath, true, partnumberQueryData.toLatin1().toHex().toUpper());
        }
        else
        {
            QMessageBox::warning(this, QString::fromLocal8Bit("����"), QString::fromLocal8Bit("��Ч�����"), QMessageBox::Yes);
            return;
        }

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

void FirmwareGenerator::switchPlatformPressed()
{
    m_leBootloaderInfo->setText("default " + m_cmbPlatformSwitch->currentText() + " boot code loaded");
}

//������generate��ť�¼�
void FirmwareGenerator::generateButtonPressed()
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

//����ʹ��Ĭ��bootloader�¼�
void FirmwareGenerator::useDefaultBootloaderPressed()
{
    if(m_ckbUseDefaultBootloader->isChecked())
    {
        m_btnLoadBootloader->setEnabled(false);
        m_leBootloaderInfo->setText("default " + m_cmbPlatformSwitch->currentText() + " boot code loaded");
        m_leBootloaderInfo->setEnabled(false);
        m_btnLoadBootloader->setStatusTip(tr("use default boot code now"));
        m_cmbPlatformSwitch->setEnabled(true);
    }
    else
    {
        m_btnLoadBootloader->setEnabled(true);
        m_leBootloaderInfo->setEnabled(true);
        m_leBootloaderInfo->clear();
        m_btnLoadBootloader->setStatusTip(tr("select the bootloader file"));
        m_cmbPlatformSwitch->setEnabled(false);
    }
}

//�������bootloader�¼�
void FirmwareGenerator::loadBootloaderPressed()
{
    QString fileName = QFileDialog::getOpenFileName();

    if(!fileName.isEmpty())
        m_leBootloaderInfo->setText(fileName);

    qDebug()<<fileName<<endl;
}

//������S021������а��س����޸İ汾�Ż�����������¼�
void FirmwareGenerator::s021ReturnPressed()
{
    QString originalS021Data = m_leDiagnosisS021->text();

    if(originalS021Data.isEmpty())
    {
        QString tmpStr = DIAG_COMMON_S0;

        //�����û����������
        bool isOK;
        QString partnumberQueryData = QInputDialog::getText(this,
                                                            QString::fromLocal8Bit("���������"),
                                                            QString::fromLocal8Bit("�����������, ����������16���ַ�"),
                                                            QLineEdit::Normal,
                                                            "",
                                                            &isOK);
        //У��������Ϣ
        QRegExp regExp("^[\\w\\-]{0,16}$");

        if(isOK && regExp.exactMatch(partnumberQueryData))
        {
            tmpStr.replace(18, partnumberQueryData.toLatin1().toHex().size(), partnumberQueryData.toLatin1().toHex());
            m_leDiagnosisS021->setText(tmpStr.toUpper());
        }
        else
        {
            QMessageBox::warning(this, QString::fromLocal8Bit("����"), QString::fromLocal8Bit("��Ч�����"), QMessageBox::Yes);
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
        QMessageBox::warning(this, QString::fromLocal8Bit("����"), QString::fromLocal8Bit("S0��������Ч"), QMessageBox::Yes);
        return;
    }

    //�����û�����汾��Ϣ����
    bool isOK;
    QString versionQueryData = QInputDialog::getText(NULL,
                                                     QString::fromLocal8Bit("��������汾��"),
                                                     QString::fromLocal8Bit("����������汾��, ���ϸ�ƥ���ʽ: xx.xx.xx\n"),
                                                     QLineEdit::Normal,
                                                     "",
                                                     &isOK);
    //У��������Ϣ, �ϸ�ƥ��xx.xx.xx
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
        QMessageBox::warning(this, QString::fromLocal8Bit("����"), QString::fromLocal8Bit("�汾����������, δ�ܸ��°汾��"), QMessageBox::Yes);
        return;
    }
}

//�������.S19�ļ��¼�
void FirmwareGenerator::selectFilePressed()
{
    //�����ļ��Ի�����
    QFileDialog* fileDialog = new QFileDialog(this);
    //�����ļ��Ի������
    fileDialog->setWindowTitle(tr("choose .S19 file"));
    //����Ĭ���ļ�·��
    fileDialog->setDirectory(".");
    //�����ļ�������
    fileDialog->setNameFilter(tr("*.S19"));
    //���ÿ���ѡ�����ļ�,Ĭ��Ϊֻ��ѡ��һ���ļ�QFileDialog::ExistingFile
    fileDialog->setFileMode(QFileDialog::ExistingFile);
    //������ͼģʽ
    fileDialog->setViewMode(QFileDialog::Detail);
    //��ӡ����ѡ����ļ���·��
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

//�ϳɺ�bootloader�Ĺ̼�
void FirmwareGenerator::generateFirmwareWithBootloader(void)
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

    //��ȡbootloader�ļ�
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

            //ԭ����bootloader.S19�ļ����ļ�ͷ���ļ�β���ϳɵ�ʱ����Ҫ�޳�
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
        ModelData md;

        if(PLATFORM_STRING_LIST.at(M1AFL2) == m_cmbPlatformSwitch->currentText())
        {
            bootloaderCodeString = md.BOOT_CODE_M1AFL2;
        }
        else if(PLATFORM_STRING_LIST.at(T19) == m_cmbPlatformSwitch->currentText())
        {
            bootloaderCodeString = md.BOOT_CODE_T19;
        }
        else if(PLATFORM_STRING_LIST.at(S51EVFL) == m_cmbPlatformSwitch->currentText())
        {
            bootloaderCodeString = md.BOOT_CODE_S51EVFL;
        }
        else if(PLATFORM_STRING_LIST.at(A13TEV) == m_cmbPlatformSwitch->currentText())
        {
            bootloaderCodeString = md.BOOT_CODE_A13TEV;
        }
        else
        {
            qDebug() << "error occurred when loading default boot code";
        }
    }

    qDebug()<<bootloaderCodeString<<endl;

    //��ȡ.S19ԭ�ļ�
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

    //��bootloader�ϳɽ�ԭʼS19�ļ�
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

    //�洢���ɵĺ�bootloader���ļ�
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

    //WINDOWS�����£�ѡ�и��ļ�
#ifdef WIN32
    QProcess process;
    QString openFileName = newFilePathName;

    openFileName.replace("/", "\\");    //***���windows�±�Ҫ***
    process.startDetached("explorer /select," + openFileName);
#endif
}

//���ɶ�����boot code���������flash driver
void FirmwareGenerator::generateFiles(CmdType cmd, QString dir_path, bool is_open_folder, QString user_part_number)
{
    QString filePathName = dir_path;
    const char *fileContent = nullptr;
    ModelData md;

    switch(cmd)
    {
    case CMD_GEN_FLASH_DRIVER:
        filePathName += "CheryCustomizedFlashDriver.S19";
        fileContent = md.FLASH_DRIVER_NO_PART_NUMBER;
        break;
    case CMD_GEN_ERASE_EEPROM:
        filePathName += "CheryM1SeriesEraseEepromFirmware.S19";
        fileContent = md.ERASE_EEPROM_FIRMWARE;
        break;
    case CMD_GEN_M1_BOOT_CODE:
        filePathName += QString("CheryM1SeriesBootCode.S19");
        fileContent = md.BOOT_CODE_M1AFL2;
        break;
    case CMD_GEN_T1_BOOT_CODE:
        filePathName += QString("CheryT1SeriesBootCode.S19");
        fileContent = md.BOOT_CODE_T19;
        break;
    case CMD_GEN_S51EVFL_BOOT_CODE:
        filePathName += QString("CheryS51evflBootCode.S19");
        fileContent = md.BOOT_CODE_S51EVFL;
        break;
    case CMD_GEN_A13TEV_BOOT_CODE:
        filePathName += QString("CheryA13tevBootCode.S19");
        fileContent = md.BOOT_CODE_A13TEV;
        break;
    default:
        break;
    }

    QDir dir(dir_path);
    if(!dir.exists())
        dir.mkdir(dir_path);

    //ɾ����ʱ�ļ�
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

    if(CMD_GEN_FLASH_DRIVER == cmd && !user_part_number.isEmpty())
    {
        QString newFileContent(fileContent);
        newFileContent.replace(18, user_part_number.size(), user_part_number);

        out << newFileContent;
    }
    else
    {
        out << fileContent;
    }


    newFile.close();

    if(is_open_folder)
    {
#ifdef WIN32
        QProcess process;
        QString openFileName = filePathName;

        openFileName.replace("/", "\\");    //***���windows�±�Ҫ***
        process.startDetached("explorer /select," + openFileName);
#endif
    }
}

//����������ù̼�
void FirmwareGenerator::generateFirmwareForDiagnosis(void)
{
    if(m_leFileInfo->text().isEmpty())
    {
        QMessageBox::warning(this, "Warnning", "Please select a .S19 file", QMessageBox::Yes);
        return;
    }

    //��ȡ.S19ԭ�ļ�
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

    //У��ԭ�ļ��Ƿ���ȷ
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

    //У��S021������
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

    //�滻S0�У�ɾ��S10B��
    originalS19FileStringList.replace(0, m_leDiagnosisS021->text().toUpper() + '\n');
    originalS19FileStringList.removeOne(REPLACE_STRING);

    for(auto elem: originalS19FileStringList)
        qDebug() << elem << endl;

    //.S19�ļ���S2��Fx��������
    if(!sortS19Code(originalS19FileStringList))
    {
        return;
    }

    //���ɲ���CRC��������ݵ��ַ���
    QStringList crcCalcStringList = originalS19FileStringList;
    QString crcCalcString;

    //S0��S9�в�����CRC����
    if(crcCalcStringList.size() >= 2)
    {
        crcCalcStringList.pop_front();
        crcCalcStringList.pop_back();
    }

    for(QString &elem: crcCalcStringList)
    {
        if(elem.endsWith('\n'))
            elem = elem.left(elem.size() - 1);

        //��������ֽڲ�����
        elem = elem.left(elem.size() - 2);

        //S1ǰ8���ֽڲ�����
        if(elem.startsWith("S1", Qt::CaseInsensitive))
            elem = elem.right(elem.size() - 8);

        //S2ǰ10���ֽڲ�����
        if(elem.startsWith("S2", Qt::CaseInsensitive))
            elem = elem.right(elem.size() - 10);

        crcCalcString += elem;
    }

    if(crcCalcString.size() % 2 != 0)
    {
        QMessageBox::warning(this, "Warnning", "data length of crc calculating error", QMessageBox::Yes);
        return;
    }

    //�������ַ�����������֣�e.g. "91b0"->0x91 0xb0
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
        //�����û�����CRC���Ǳ��еĴ洢λ��
        bool isOK;
        QString crcAddressQueryData = QInputDialog::getText(NULL,
                                                         QString::fromLocal8Bit("����crcУ�����ַ"),
                                                         QString::fromLocal8Bit("������crcУ������оƬ�еĴ洢��ַ, ����6λ\n16�����ַ����(һ��ΪFE8000��FEBE00)"),
                                                         QLineEdit::Normal,
                                                         "",
                                                         &isOK);
        //У��������Ϣ
        QRegExp regExp("^[a-fA-F\\d]{6}$");

        if(isOK && regExp.exactMatch(crcAddressQueryData))
        {
            s20cText = "S20C" + crcAddressQueryData.toUpper();
        }
        else
        {
            QMessageBox::warning(this, QString::fromLocal8Bit("����"), QString::fromLocal8Bit("����ĵ�ַ��Ч"), QMessageBox::Yes);
            return;
        }
    }

    QMessageBox::information(this, QString::fromLocal8Bit("�̼���Ϣ"),
                             QString::fromLocal8Bit("�����: ") + partNumber + QString::fromLocal8Bit(", ����汾��: ") \
                             + softwareVersion + QString::fromLocal8Bit(", crcУ����洢��ַ: 0x") + s20cText.right(6),
                             QMessageBox::Yes);
    qDebug() << "crc address: " << s20cText;

    QString crcStr = QString::number(crc, 16);
    for(int i = 0, s = crcStr.size(); i < 4 - s; ++i)
        crcStr = '0' + crcStr;

    qDebug() << "crc string: " << crcStr;

    for(int cnt = 0; cnt < 4; ++cnt)
        s20cText.append(crcStr);

    if(chkSum <= 0x0f)
        s20cText += "0"+ QString::number(chkSum, 16);
    else
        s20cText += QString::number(chkSum, 16);

    if(s20cText.size() != 28)
    {
        QMessageBox::warning(this, QString::fromLocal8Bit("����"), QString::fromLocal8Bit("����s20c���ַ��������ַ�����Ӧ��28"), QMessageBox::Yes);
        return;
    }

    s20cText = s20cText.toUpper();
    m_leDiagnosisS20C->setText(s20cText);

    qDebug() << "s20cText: " << s20cText;

    //��S20C������ӻ��з���д��������ɵ��ļ��ĵ����ڶ���
    s20cText.append('\n');
    originalS19FileStringList.insert(originalS19FileStringList.size() - 1, s20cText);

    //���ɹ̼������ļ����ж�λ���ļ�
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

    //ͬʱ����һ��flash driver
    generateFiles(CMD_GEN_FLASH_DRIVER, dirPath, false, m_leDiagnosisS021->text().right(m_leDiagnosisS021->text().size() - 18).left(32));

    //windowsϵͳ��ֱ�Ӵ򿪸��ļ��в�ѡ�������app�ļ�
#ifdef WIN32
    QProcess process;
    QString openNewFileName = diagnosisFilePathName;

    openNewFileName.replace("/", "\\");    //���windows�±�Ҫ
    process.startDetached("explorer /select," + openNewFileName);
#endif
}

//��.S19ԭ�ļ������S224����Σ����ڴ��ҳ����˳���������У�F1��F2����������FF��
bool FirmwareGenerator::sortS19Code(QStringList &originalStringList)
{
    QStringList stringListS0AndS9;
    QStringList stringListS1;
    QStringList stringListS2[16]; //F0-FF���16��

    //S0��S9��
    stringListS0AndS9.push_back(originalStringList.first());
    stringListS0AndS9.push_back(originalStringList.last());

    for(QString elem: originalStringList)
    {
        //S1�����
        if(elem.startsWith("S1", Qt::CaseInsensitive))
        {
            stringListS1.push_back(elem);
        }

        if(elem.startsWith("S2", Qt::CaseInsensitive))
        {
            //S2����Σ���ͷΪ��S2**Fx, x��0-F֮��
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

    //����originalStringList,��S0-S1-S2-S9��˳��
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

//�����ַ��洢��ʮ����������ת��Ϊ��Ӧ����
int FirmwareGenerator::hexCharToHex(char src)
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

//����CRC���ο���ECU bootloader and programming implementation specification��
unsigned short FirmwareGenerator::calcCRC16(QList<unsigned char> data_list)
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

//����S20C�е�checksum
unsigned char FirmwareGenerator::calcChecksum(unsigned short crc)
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

//����seed����ó�keyֵ
unsigned short FirmwareGenerator::calculateKey(unsigned short seed)
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

//����seed����ó�keyֵ
void FirmwareGenerator::dealWithCalculateKeyCommand(void)
{
    showHelpInfo(CMD_HELP_DIAG_CALCULATE_KEY);

    //�����û������������
    bool isOK;
    QString seedQueryData = QInputDialog::getText(NULL,
                                                  QString::fromLocal8Bit("��������"),
                                                  QString::fromLocal8Bit("��������4λ16�����ַ���ɵ�����"),
                                                  QLineEdit::Normal,
                                                  "",
                                                  &isOK);

    //ɾ������Ŀո�ȿհ׷�
    seedQueryData.remove(QRegExp("\\s"));
    qDebug() << seedQueryData;

    //У��������Ϣ
    QRegExp regExp("^[0-9a-fA-F]{4}$");

    if(isOK && regExp.exactMatch(seedQueryData))
    {
        unsigned short seed = 0;
        unsigned short key = 0;

        bool resultOK = false;
        seed = seedQueryData.toUShort(&resultOK, 16);

        if(!resultOK)
        {
            QMessageBox::warning(this, QString::fromLocal8Bit("����"), QString::fromLocal8Bit("���Ӽ���ʧ��"), QMessageBox::Yes);
        }
        else
        {
            key = calculateKey(seed);
            QString keyHexStr = QString::number(key, 16).toUpper();

            if(keyHexStr.size() > 4)
            {
                QMessageBox::warning(this, QString::fromLocal8Bit("����"), QString::fromLocal8Bit("��Կ���Ȳ��ܳ���4"), QMessageBox::Yes);
                return;
            }

            //�������ʮ�������ַ���С��4������ǰ�油0������Ҫ������4���ַ�
            for(int cnt = 0; cnt < 4 - keyHexStr.size(); ++cnt)
                keyHexStr.prepend('0');

            qDebug() << keyHexStr;

            QClipboard *clipboard = QApplication::clipboard();
            clipboard->clear();
            clipboard->setText("2704" + keyHexStr);

            ptOutputWnd->clear();
            ptOutputWnd->appendPlainText("*************************************");
            ptOutputWnd->appendPlainText(QString::fromLocal8Bit("   ����: [") + seedQueryData.toUpper() + QString::fromLocal8Bit("]  ->  ��Կ: [") + keyHexStr + "]");
            ptOutputWnd->appendPlainText("*************************************");
            ptOutputWnd->appendPlainText(QString::fromLocal8Bit("����Ѿ�������ϵͳ��������:[2704") + keyHexStr + "]");
            ptOutputWnd->appendPlainText("*************************************");

            QMessageBox::information(this, QString::fromLocal8Bit("��Կ������"),
                                     QString::fromLocal8Bit("ʮ��������Կ������Ϊ��") + keyHexStr + QString::fromLocal8Bit("��, ����Ѿ�������ϵͳ��������"),
                                     QMessageBox::Yes);
        }
    }
    else
    {
        QMessageBox::warning(this, QString::fromLocal8Bit("����"), QString::fromLocal8Bit("��Ч����"), QMessageBox::Yes);
    }
}

void FirmwareGenerator::switchFunctionPage(FirmwareGenerator::FunctionType functype)
{
    if(functype <= CMD_HANDLER)
    {
        m_cmbFunctionSwitch->setCurrentIndex(functype);
    }
    else if(CMD_HANDLER_CALCULATE_KEY == functype)
    {
        m_cmbFunctionSwitch->setCurrentIndex(CMD_HANDLER);
        m_leRunCommand->setText(":ck");
    }
    else if(CMD_HANDLER_COMPRESS_ARRAY_OF_BMP == functype)
    {
        m_cmbFunctionSwitch->setCurrentIndex(CMD_HANDLER);
        m_leRunCommand->setText(":cb");
    }
}

//������Ϣ
void FirmwareGenerator::showHelpInfo(CmdType cmd)
{
    QStringList hlpInfo;

    if(CMD_HELP == cmd)
    {
        hlpInfo << QString::fromLocal8Bit("�������б�");
        hlpInfo << QString::fromLocal8Bit("0 ��ӭʹ�ñ��������ҳ��������֧�ֵ�ָ����书�ܶ���.");
        hlpInfo << QString::fromLocal8Bit("0.1 ��<u>switch function</u>����<u>input command</u>���л���<u>run command</u>��"
                                          "��������������<u>:help</u>��<u>:hlp</u>��<u>:?</u>�󰴻س������뱾ҳ��.");
        hlpInfo << QString::fromLocal8Bit("0.2 ���������ʹ�÷���ͬ0.1�ڣ�<u>ָ�����벻���ִ�Сд</u>.");

        hlpInfo << QString::fromLocal8Bit("1 ���������.");
        hlpInfo << QString::fromLocal8Bit("1.1 ����.");
        hlpInfo << QString::fromLocal8Bit("1.1.1 ���壺��ձ������Ļ��ǰ������ʾ������.");
        hlpInfo << QString::fromLocal8Bit("1.1.2 ָ�<u>:clear screen</u>��<u>:cs</u>.");
        hlpInfo << QString::fromLocal8Bit("1.2 ȫ����ʾ.");
        hlpInfo << QString::fromLocal8Bit("1.2.1 ���壺������л���ȫ����ʾ.");
        hlpInfo << QString::fromLocal8Bit("1.2.2 ָ�<u>:full screen</u>��<u>:fs</u>.");
        hlpInfo << QString::fromLocal8Bit("1.3 ������ʾ.");
        hlpInfo << QString::fromLocal8Bit("1.3.1 ���壺������л���������С��ʾ.");
        hlpInfo << QString::fromLocal8Bit("1.3.2 ָ�<u>:normal screen</u>��<u>:ns</u>.");
        hlpInfo << QString::fromLocal8Bit("1.4 ��������.");
        hlpInfo << QString::fromLocal8Bit("1.4.1 ���壺���汾�����������������Ŀ¼������config.json�ļ�.");
        hlpInfo << QString::fromLocal8Bit("1.4.2 ָ�<u>:save config file</u>��<u>:scf</u>.");
        hlpInfo << QString::fromLocal8Bit("1.4.3 ��ע����ǰ��ʵ�ֽӿڣ���δ��ʵ����Ҫ�洢��������.");
        hlpInfo << QString::fromLocal8Bit("1.5 ��������.");
        hlpInfo << QString::fromLocal8Bit("1.5.1 ���壺���ر��������������������Ŀ¼�µ�config.json�ļ�.");
        hlpInfo << QString::fromLocal8Bit("1.5.2 ָ�<u>:load config file</u>��<u>:lcf</u>.");
        hlpInfo << QString::fromLocal8Bit("1.5.3 ��ע����ǰ��ʵ�ֽӿڣ���δ��ʵ����Ҫ���ص�������.");
        hlpInfo << QString::fromLocal8Bit("1.6 �˳�����.");
        hlpInfo << QString::fromLocal8Bit("1.6.1 ���壺�˳������.");
        hlpInfo << QString::fromLocal8Bit("1.6.2 ָ�<u>:quit</u>��<u>:q</u>.");

        hlpInfo << QString::fromLocal8Bit("2 Windowsϵͳ���߿�ݿ�������.");
        hlpInfo << QString::fromLocal8Bit("2.1 ����Windowsϵͳָ��ͨ�˷���.");
        hlpInfo << QString::fromLocal8Bit("2.1.1 ���壺��Windowsϵͳ�Դ��ġ����С����һ������Windowsϵͳָ��.");
        hlpInfo << QString::fromLocal8Bit("2.1.2 ָ�<u>::command</u>������<u>command</u>ΪWindowsϵͳ��ʶ���ָ��.");
        hlpInfo << QString::fromLocal8Bit("2.1.3 ��ע��������QT�����л��ƣ�����Windowsϵͳָ����ܲ��ᱻ��ȷִ��.");
        hlpInfo << QString::fromLocal8Bit("2.2 ������.");
        hlpInfo << QString::fromLocal8Bit("2.2.1 ���壺���ٿ���Windowsϵͳ�Դ��ġ������������.");
        hlpInfo << QString::fromLocal8Bit("2.2.2 ָ�<u>:calculator</u>��<u>:calc</u>��<u>:c</u>.");
        hlpInfo << QString::fromLocal8Bit("2.3 ���������.");
        hlpInfo << QString::fromLocal8Bit("2.3.1 ���壺���ٿ���Windowsϵͳ�Դ��ġ���������������.");
        hlpInfo << QString::fromLocal8Bit("2.3.2 ָ�<u>:taskmgr</u>��<u>:tm</u>.");
        hlpInfo << QString::fromLocal8Bit("2.4 ��ͼ.");
        hlpInfo << QString::fromLocal8Bit("2.4.1 ���壺���ٿ���Windowsϵͳ�Դ��ġ���ͼ�����.");
        hlpInfo << QString::fromLocal8Bit("2.4.2 ָ�<u>:mspaint</u>��<u>:mp</u>.");

        hlpInfo << QString::fromLocal8Bit("3 ����Ϊ�������߹���.");
        hlpInfo << QString::fromLocal8Bit("3.1 bootloader�ϳ�.");
        hlpInfo << QString::fromLocal8Bit("3.1.1 ���壺����M1ϵ��bootloader�ϳɷ�������.");
        hlpInfo << QString::fromLocal8Bit("3.1.2 ָ�<u>:bootloader?</u>��<u>:b?</u>.");
        hlpInfo << QString::fromLocal8Bit("3.2 �����app�ϳ�.");
        hlpInfo << QString::fromLocal8Bit("3.2.1 ���壺����M1ϵ�������app�ϳɷ�������.");
        hlpInfo << QString::fromLocal8Bit("3.2.2 ָ�<u>:diagnosis?</u>��<u>:d?</u>.");
        hlpInfo << QString::fromLocal8Bit("3.3 ����flash driver�ļ�.");
        hlpInfo << QString::fromLocal8Bit("3.3.1 ���壺��������һ���������flash driver�ļ�.");
        hlpInfo << QString::fromLocal8Bit("3.3.2 ָ�<u>:flash driver</u>��<u>:fd</u>.");
        hlpInfo << QString::fromLocal8Bit("3.3.3 ��ע���˹�����Ҫ������������ţ���3.2�ڷ���Ҳ���Զ�����flash driver�ļ�.");
        hlpInfo << QString::fromLocal8Bit("3.4 ����erase eeprom firmware�ļ�.");
        hlpInfo << QString::fromLocal8Bit("3.4.1 ���壺������������Ǳ�eeprom��.S19�̼�.");
        hlpInfo << QString::fromLocal8Bit("3.4.2 ָ�<u>:erase eeprom</u>��<u>:ee</u>.");
        hlpInfo << QString::fromLocal8Bit("3.4.3 ��ע����¼�˹̼��������Ǳ�ԭ����app�̼�����Ҫ������¼app�̼�.");
        hlpInfo << QString::fromLocal8Bit("3.5 ����M1ϵ��boot code�ļ�.");
        hlpInfo << QString::fromLocal8Bit("3.5.1 ���壺����M1ϵ��boot����ε�.S19�̼�.");
        hlpInfo << QString::fromLocal8Bit("3.5.2 ָ�<u>:m boot code</u>��<u>:mbc</u>.");
        hlpInfo << QString::fromLocal8Bit("3.5.3 ��ע���˹̼����ɵ�����¼���Ǳ���Ҫ�ϳɵ��Ǳ�app�̼��У��ϳɷ����ο�3.1��.");
        hlpInfo << QString::fromLocal8Bit("3.6 ����T1ϵ��boot code�ļ�.");
        hlpInfo << QString::fromLocal8Bit("3.6.1 ���壺����T1ϵ��boot����ε�.S19�̼�.");
        hlpInfo << QString::fromLocal8Bit("3.6.2 ָ�<u>:t boot code</u>��<u>:tbc</u>.");
        hlpInfo << QString::fromLocal8Bit("3.6.3 ��ע���˹̼����ɵ�����¼���Ǳ���Ҫ�ϳɵ��Ǳ�app�̼��У��ϳɷ����ο�3.1��.");
        hlpInfo << QString::fromLocal8Bit("3.7 ����S51EVFL��boot code�ļ�.");
        hlpInfo << QString::fromLocal8Bit("3.7.1 ���壺����S51EVFL��boot����ε�.S19�̼�.");
        hlpInfo << QString::fromLocal8Bit("3.7.2 ָ�<u>:s boot code</u>��<u>:sbc</u>.");
        hlpInfo << QString::fromLocal8Bit("3.7.3 ��ע���˹̼����ɵ�����¼���Ǳ���Ҫ�ϳɵ��Ǳ�app�̼��У��ϳɷ����ο�3.1��.");
        hlpInfo << QString::fromLocal8Bit("3.8 ����A13TEV��boot code�ļ�.");
        hlpInfo << QString::fromLocal8Bit("3.8.1 ���壺����A13TEV��boot����ε�.S19�̼�.");
        hlpInfo << QString::fromLocal8Bit("3.8.2 ָ�<u>:a boot code</u>��<u>:abc</u>.");
        hlpInfo << QString::fromLocal8Bit("3.8.3 ��ע���˹̼����ɵ�����¼���Ǳ���Ҫ�ϳɵ��Ǳ�app�̼��У��ϳɷ����ο�3.1��.");

        hlpInfo << QString::fromLocal8Bit("4 ������������.");
        hlpInfo << QString::fromLocal8Bit("4.1 �ļ�ת�ַ�������.");
        hlpInfo << QString::fromLocal8Bit("4.1.1 ���壺��ָ���ļ��������ֽ�����C/C++������ʶ������飬���洢Ϊ.h�ļ�.");
        hlpInfo << QString::fromLocal8Bit("4.1.2 ָ�<u>:convert code to sQString::fromLocal8Biting</u>��<u>:c2s</u>.");
        hlpInfo << QString::fromLocal8Bit("4.2 �������Ӽ�����Կ.");
        hlpInfo << QString::fromLocal8Bit("4.2.1 ���壺ĳЩ��Ϸ�����Ҫ�����������ܸ�������������������Ӧ����Կ.");
        hlpInfo << QString::fromLocal8Bit("4.2.2 ָ�<u>:calculate key</u>��<u>:calc key</u>��<u>:ck</u>.");
        hlpInfo << QString::fromLocal8Bit("4.2.3 ��ע������<u>:calculate key?</u>��<u>:calc key?</u>��<u>:ck?</u>����ʾ��ȫ�����������������Ϣ.");
        hlpInfo << QString::fromLocal8Bit("4.3 ��Img2Lcd���ɵ�λͼC��������ѹ������.");
        hlpInfo << QString::fromLocal8Bit("4.3.1 ���壺��Img2Lcd���ɵ�λͼC�������ݣ������е�ȫ��0x00��0xff�����ճ��ֵĴ�������ѹ��.");
        hlpInfo << QString::fromLocal8Bit("4.3.2 ָ�<u>:compress bmp</u>��<u>:cb</u>.");
    }
    else if(CMD_HELP_BOOTLOADER == cmd)
    {
        hlpInfo << QString::fromLocal8Bit("��BootLoader�ϳɹ���ʹ�÷�����");
        hlpInfo << QString::fromLocal8Bit("0 ��<u>switch function</u>����<u>input command</u>���л���<u>add bootloader to firmware</u>.");
        hlpInfo << QString::fromLocal8Bit("1 ����bootloader����ε����ַ�ʽ.");
        hlpInfo << QString::fromLocal8Bit("1.1 ��ʽһ������Ĭ��bootloader����Σ��ȹ�ѡ<u>use default</u>���ٸ�������������ѡ�����ѡ������ض��ͺŵ�Ĭ��bootloader�����.");
        hlpInfo << QString::fromLocal8Bit("1.2 ��ʽ������������bootloader����Σ�ȥ��ѡ<u>default</u>�����<u>load bootloader</u>��ťѡ������bootloader�ļ�.");
        hlpInfo << QString::fromLocal8Bit("2 ���<u>load file</u>��ťѡ��.S19ԭapp�ļ�.");
        hlpInfo << QString::fromLocal8Bit("3 ���<u>generate</u>��ť���ɺ�bootloader����app�ļ�,���Զ��򿪸��ļ����ڵ�Ŀ¼��ѡ�и��ļ�.");
    }
    else if(CMD_HELP_DIAGNOSIS == cmd)
    {
        hlpInfo << QString::fromLocal8Bit("�������app���ɹ���ʹ�÷�����");
        hlpInfo << QString::fromLocal8Bit("0 ��<u>switch function</u>����<u>input command</u>���л���<u>gen firmware for diagnosis</u>.");
        hlpInfo << QString::fromLocal8Bit("1 ���<u>load file</u>��ťѡ��.S19ԭapp�ļ�.");
        hlpInfo << QString::fromLocal8Bit("2 ����S021���ݣ����������ս�λ��app�ĵ�һ��.");
        hlpInfo << QString::fromLocal8Bit("2.1 ��S021���������<u>:t</u>�����س�����ȡT19Ԥ�õ�S021���ݣ�����<u>:m</u>�����س�����ȡm1afl2Ԥ�õ�S021���ݣ�"
                      "����<u>:s</u>�����س�����ȡs51evflԤ�õ�S021���ݣ�����<u>:a</u>�����س�����ȡa13tevԤ�õ�S021����.");
        hlpInfo << QString::fromLocal8Bit("2.2 ��ȷ����S021���ݺ󣬽��������S021�������ڵ������󣬵���س��������޸İ汾�ţ��汾�Ÿ�ʽ���ϸ�ƥ��<u>xx.xx.xx</u>,xΪ0-9��a-f,��ĸ�����ִ�Сд�����հ���д��ĸд���ļ�.");
        hlpInfo << QString::fromLocal8Bit("2.3 ���������S021�������ڵ������󣬵�S021�����Ϊ��ʱ���»س�������������������ţ�֮���������������汾��Ϣ��Ȼ���Զ��ϳ�S021�����ݣ��������ɺ���Ȼ����ʹ��2.1�ڵķ����޸İ汾��.");
        hlpInfo << QString::fromLocal8Bit("3 ���<u>generate</u>��ť�����������app�ļ�,���Զ��򿪸��ļ����ڵ�Ŀ¼��ѡ�и��ļ�.");
        hlpInfo << QString::fromLocal8Bit("4 ���ļ����»����Զ�����flash driver�ļ����뽫�����app�ļ���flash driver�ļ�һͬ����ѹ�����ṩ��ʹ����.");
    }
    else if(CMD_HELP_DIAG_CALCULATE_KEY == cmd)
    {
        hlpInfo << QString::fromLocal8Bit("����ȫ�����������衷");
        hlpInfo << QString::fromLocal8Bit("0 ��busmaster�����ϴ��ڽ�[Send Tester Present]��ѡΪ[ON]������3E����.");
        hlpInfo << QString::fromLocal8Bit("1 ��busmaster�����ϴ��ڷ���[10 03]���л�����չ�Ự.");
        hlpInfo << QString::fromLocal8Bit("2 ��busmaster�����ϴ��ڷ���[27 03]��������㰲ȫ��Կ������.");
        hlpInfo << QString::fromLocal8Bit("3 ��busmaster�����ϴ����յ���[67 03 xx xx]�еĺ������ֽڼ�[xx xx]���뵽������[seed query]������.");
        hlpInfo << QString::fromLocal8Bit("4 ���[seed query]���ڵ�[OK]���᷵�ؼ���õİ�ȫ��Կ.");
        hlpInfo << QString::fromLocal8Bit("5 ������Կ.");
        hlpInfo << QString::fromLocal8Bit("5.1 ����һ����busmaster�����ϴ��ڷ���[27 04 hh hh]������[hh hh]Ϊ��4�����õ���ֵ.");
        hlpInfo << QString::fromLocal8Bit("5.2 ����������4����ɺ󣬳����Ѿ���������Ƶ�ϵͳ�������У���busmaster�����Ϸ��ʹ��ڣ�ʹ������Ҽ�������ѡ��ճ����Ȼ���ͼ���.");
    }

    ptOutputWnd->clear();
    foreach(QString elem, hlpInfo)
    {
        //����һ���¶���
        if(elem.startsWith("0 ") || elem.startsWith("1 ") || elem.startsWith("2 ") || elem.startsWith("3 ") || elem.startsWith("4 ") || elem.startsWith("5 "))
        {
            ptOutputWnd->appendPlainText(QString());
        }

        ptOutputWnd->appendHtml(elem);
    }

    //����ƶ��������������ʾ�ļ�ĩβ
    QTextCursor cursor = ptOutputWnd->textCursor();
    cursor.movePosition(QTextCursor::Start);
    ptOutputWnd->setTextCursor(cursor);
}

//���������ļ�
void FirmwareGenerator::procConfigFile(CmdType cmd)
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

void FirmwareGenerator::autoUpdateTypeB()
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

void FirmwareGenerator::versionDetectTimerTimeout()
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

        if(!fileStringList.isEmpty() && fileStringList.at(0) > SOFTWARE_VERSION && fileStringList.at(0).startsWith('v'))
        {
            int ret = QMessageBox::question(this, tr("�Զ�����"), "��⵽�°汾" + fileStringList.at(0) + "����ǰ�汾" + SOFTWARE_VERSION + "���Ƿ�������", QMessageBox::Yes, QMessageBox::No);
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

void FirmwareGenerator::appDetectTimerTimeout()
{
    static int howmany1sPassed = 0;
    static bool isDownloadSuccess = false;
    ++howmany1sPassed;

    qDebug() << "howmany1sPassed(app): " << howmany1sPassed;
//    ptOutputWnd->appendPlainText("howmany1sPassed(app): " + QString::number(howmany1sPassed));

    if(300 == howmany1sPassed && !isDownloadSuccess)
    {
        QMessageBox::critical(NULL, tr("�Զ�����"), tr("���¿���ʧ����..."));
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
            int ret = QMessageBox::information(this, tr("�Զ�����"), "���³ɹ����Ƿ�鿴�汾������־��", QMessageBox::Yes, QMessageBox::No);
            if(QMessageBox::Yes == ret)
                QDesktopServices::openUrl(QUrl(QLatin1String("https://github.com/bingshuizhilian/QTPROJECTS-FIRMWARE_GENERATOR/releases")));

            //WINDOWS�����£�ѡ�и��ļ�
#ifdef WIN32
            QProcess process;
            QString openFileName = appFilePathName;

            openFileName.replace("/", "\\");    //***���windows�±�Ҫ***
            process.startDetached("explorer /select," + openFileName);
#endif
        }
        else
        {
            appFile.remove();
            QMessageBox::critical(NULL, tr("�Զ�����"), "����ʧ����...");
        }
    }
}

//����QT��������������ʵ�����紫�䣬���ǲ�������һЩ��Ҫ�����ӣ���������ʱ���ٷ���
void FirmwareGenerator::autoUpdate(QString local_version)
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

    //���������ϵ�����汾��
    m_networkAccessMngr = new QNetworkAccessManager;
    QUrl url(VERSION_DOWNLOAD_URL);
    QNetworkRequest request(url);
//    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/octet-stream");

    m_httpReply = m_networkAccessMngr->get(request);//��������

    connect(m_httpReply, SIGNAL(readyRead()), this, SLOT(httpReadContent()));
    connect(m_networkAccessMngr, SIGNAL(finished(QNetworkReply*)), this, SLOT(httpReplyFinished(QNetworkReply*)));
    connect(m_httpReply, SIGNAL(error(QNetworkReply::NetworkError)),this,SLOT(httpDownloadError(QNetworkReply::NetworkError)));

    //ѯ���Ƿ���Ҫ����
//    int ret = QMessageBox::question(this, tr("�Զ�����"), tr("��Ҫ���µ����°汾������"), QMessageBox::Yes, QMessageBox::No);
//    if(QMessageBox::No == ret)
//        return;

    //���ط������ϵ����°汾���


}

void FirmwareGenerator::httpReadContent()
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

void FirmwareGenerator::httpReplyFinished(QNetworkReply *reply)
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

void FirmwareGenerator::httpDownloadError(QNetworkReply::NetworkError error)
{
    qDebug() << "httpDownloadError: " << error;
}

void FirmwareGenerator::httpDownloadProgress(qint64 bytes_received, qint64 bytes_total)
{
    qDebug() << "httpDownloadProgress: " << bytes_received << bytes_total << QString(" -> %1%").arg(bytes_received * 100 / bytes_total);
}

//��boot code����Ϊ�ַ�����������boot code����ʱ�����ô˺�������ת��Ϊ����
void FirmwareGenerator::generateCharArray()
{
    ptOutputWnd->clear();
    ptOutputWnd->appendPlainText(QString::fromLocal8Bit("��ӭʹ���ļ�ת�ַ�������"));

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

    //WINDOWS�����£�ѡ�и��ļ�
#ifdef WIN32
    QProcess process;
    QString openFileName = newFilePathName;

    openFileName.replace("/", "\\");    //***���windows�±�Ҫ***
    process.startDetached("explorer /select," + openFileName);
#endif
}

void FirmwareGenerator::compressCArrayOfBitmap()
{
    ptOutputWnd->clear();
    ptOutputWnd->appendPlainText(QString::fromLocal8Bit("��ӭʹ��Image2Lcd���ɵ�C���������ѹ������"));

    const int DEVIDE_AMOUNT = 255; //��λ���洢��͸߸���һ���ֽڣ��˴�Ӧȷ����ֵ������255����0xff��
    QString filePathName = QFileDialog::getOpenFileName(this, QString::fromLocal8Bit("��ѡ����Image2Lcd���ɵ�λͼ�����ļ�"),
                                                        "",
                                                        tr("C files (*.c *.h)"));
    QString filePath = QFileInfo(filePathName).absolutePath();

    qDebug()<<filePathName<<endl<<filePath<<endl;

    if(filePathName.isEmpty())
    {
        QMessageBox::warning(this, QString::fromLocal8Bit("����"), QString::fromLocal8Bit("δѡ���ļ�, ѹ��ʧ��"), QMessageBox::Yes);
        return;
    }

    QFile targetFile(filePathName);
    if(!targetFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::warning(this, QString::fromLocal8Bit("����"), QString::fromLocal8Bit("�޷���") + filePathName, QMessageBox::Yes);
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

    //ע��3.5" TFTʵ��ʹ�õ���ͼƬ�ߴ������ⷽ���ϲ����Ҳ��ܳ���248(���������������λ�������������������)
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
    //�ڽ�β���һ��������ǣ���������ȫΪ0x00��0xff���������ϴ�ʱ�������������ʽ���ҵ�һ����Ϊ0x00��0xff�����ݷǳ���ʱ
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
                        //�Ƴ�����ӵ�0x00��0xff
                        targetStringList.removeLast();
                    }
                }

                //forѭ�����ϻ���index����1����ǰ��ȥ1����������ܶ�λ����һ����0x00��0xff��Ԫ�أ���matchIndex��ָ���Ԫ��
                index = matchIndex - 1;
            }
        }
        else
        {
            targetStringList << tmpStringList.at(index);
        }
    }

    //�Ƴ����õ�"END_FLAG"
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

    //�Ƴ����һ��", "��","��(�϶�Ϊ����֮һ��������remove���ɱ�֤)
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

    //��Ⱥ͸߶���Сд��ĸ
    if(!targetFormattedStringList.isEmpty())
        targetFormattedStringList[1] = targetFormattedStringList[1].left(14) + targetFormattedStringList[1].right(targetFormattedStringList[1].size() - 14).toLower();

    for(auto& elem: targetFormattedStringList)
        ptOutputWnd->appendPlainText(elem);

    QClipboard *clipboard = QApplication::clipboard();
    clipboard->clear();
    clipboard->setText(ptOutputWnd->document()->toPlainText());

    ptOutputWnd->appendPlainText(QString::fromLocal8Bit("\n\n***��������Ѿ�������ϵͳ��������***"));
}

//�ؼ���ʼ��
void FirmwareGenerator::componentsInitialization(void)
{
    setWindowTitle(tr("CheryFirmwareGenerator"));

    //�������Ƽ�״̬������
    auto labelAuthorInfo = new QLabel;
    labelAuthorInfo->setStatusTip(tr("click to view source code on github"));
    labelAuthorInfo->setOpenExternalLinks(true);
    labelAuthorInfo->setText(QString::fromLocal8Bit("<style> a {text-decoration: none} </style> <a href = https://www.github.com/bingshuizhilian/QTPROJECTS-ICM-DEVELOPMENT-TOOLBOX> contact author </a>"));
    labelAuthorInfo->show();
    ui->statusBar->addPermanentWidget(labelAuthorInfo);

    //ѡ��.S19�ļ���ť
    m_btnChooseFile = new QPushButton(tr("load file"));
    connect(m_btnChooseFile, &m_btnChooseFile->clicked, this, &selectFilePressed);
    m_btnChooseFile->setStatusTip(tr("select the original .S19 file"));
    //��ʾ.S19�ļ���
    m_leFileInfo = new QLineEdit;
    m_leFileInfo->setReadOnly(true);

    //ѡ��bootloader�ļ���ť
    m_btnLoadBootloader = new QPushButton(tr("load bootloader"));
    connect(m_btnLoadBootloader, &m_btnLoadBootloader->clicked, this, &loadBootloaderPressed);
    //��ʾbootloader�ļ���������ʾ����ʹ��Ĭ��boot code
    m_leBootloaderInfo = new QLineEdit;
    m_leBootloaderInfo->setReadOnly(true);

    //����ѡ��
    m_cmbPlatformSwitch = new QComboBox;
    connect(m_cmbPlatformSwitch, static_cast<void (QComboBox::*)(const QString&)>(&QComboBox::currentTextChanged), this, &switchPlatformPressed);
    m_cmbPlatformSwitch->setFixedWidth(77);
    m_cmbPlatformSwitch->setInsertPolicy(QComboBox::NoInsert);
    m_cmbPlatformSwitch->setStatusTip("select a platform");
    m_cmbPlatformSwitch->addItem(PLATFORM_STRING_LIST.at(M1AFL2), M1AFL2);
    m_cmbPlatformSwitch->addItem(PLATFORM_STRING_LIST.at(T19), T19);
    m_cmbPlatformSwitch->addItem(PLATFORM_STRING_LIST.at(S51EVFL), S51EVFL);
    m_cmbPlatformSwitch->addItem(PLATFORM_STRING_LIST.at(A13TEV), A13TEV);

    //�Ƿ�ʹ��Ĭ��boot codeѡ��
    m_ckbUseDefaultBootloader = new QCheckBox(tr("use default"));
    connect(m_ckbUseDefaultBootloader, &m_ckbUseDefaultBootloader->stateChanged, this, &useDefaultBootloaderPressed);
    m_ckbUseDefaultBootloader->setStatusTip(tr("use default boot code when checked"));
    m_ckbUseDefaultBootloader->setChecked(true);

    //���ɰ�ť
    m_btnGenerate = new QPushButton(tr("generate"));
    connect(m_btnGenerate, &m_btnGenerate->clicked, this, &generateButtonPressed);

    //�ϳ������firmwareʱ��Ҫ�Ķ�����Ϣ
    m_leDiagnosisS021 = new QLineEdit;
    connect(m_leDiagnosisS021, &m_leDiagnosisS021->returnPressed, this, &s021ReturnPressed);
    m_leDiagnosisS021->setStatusTip("press enter to modify version");
    m_leDiagnosisS021->setPlaceholderText("press enter for part number or sw version");
    m_leDiagnosisS20C = new QLineEdit;
    m_leDiagnosisS20C->setReadOnly(true);
    m_leDiagnosisS20C->setStatusTip("crc and checksum infos");
    m_leDiagnosisS20C->setPlaceholderText("this line is automatically filled");

    //�����������������
    m_leRunCommand = new QLineEdit;
    connect(m_leRunCommand, &m_leRunCommand->returnPressed, this, &runCmdReturnPressed);
    m_leRunCommand->setStatusTip("press enter to run command");
    m_leRunCommand->setPlaceholderText("input :? for help");
    ptOutputWnd = new QPlainTextEdit;
    ptOutputWnd->setReadOnly(true);
    ptOutputWnd->setStatusTip(tr("execute result echo window"));

    //���ֿؼ�
    m_gbBootloader = new QGroupBox;
    m_gbBootloader->setTitle(tr("bootloader settings"));
    m_gbS19Selector = new QGroupBox;
    m_gbS19Selector->setTitle(tr("select .S19 file"));
    m_gbDiagnosis = new QGroupBox;
    m_gbDiagnosis->setTitle(tr("diagnosis data settings"));
    m_gbSwitchFunction = new QGroupBox;

    //����ѡ��������
    m_cmbFunctionSwitch = new QComboBox;
    connect(m_cmbFunctionSwitch, static_cast<void (QComboBox::*)(const QString&)>(&QComboBox::currentTextChanged), this, &switchFunctionPressed);
    m_cmbFunctionSwitch->setInsertPolicy(QComboBox::NoInsert);
    m_cmbFunctionSwitch->setStatusTip("select a function");
    m_cmbFunctionSwitch->addItem(FUNCTION_STRING_LIST.at(BOOTLOADER), FUNCTION_STRING_LIST.at(BOOTLOADER));
    m_cmbFunctionSwitch->addItem(FUNCTION_STRING_LIST.at(DIAGNOSIS), FUNCTION_STRING_LIST.at(DIAGNOSIS));
    m_cmbFunctionSwitch->addItem(FUNCTION_STRING_LIST.at(CMD_HANDLER), FUNCTION_STRING_LIST.at(CMD_HANDLER));
}

//���ֳ�ʼ��
void FirmwareGenerator::layoutsInitialization()
{
    //bootloader�ؼ���
    auto layoutBootloader = new QGridLayout;
    auto labelLeBootloader = new QLabel(tr("bootloader"));
    layoutBootloader->addWidget(labelLeBootloader, 0, 0, 1, 1);
    layoutBootloader->addWidget(m_leBootloaderInfo, 0, 1, 1, 2);
    layoutBootloader->addWidget(m_cmbPlatformSwitch, 1, 0, 1, 1);
    layoutBootloader->addWidget(m_ckbUseDefaultBootloader, 1, 1, 1, 1);
    layoutBootloader->addWidget(m_btnLoadBootloader, 1, 2, 1, 1);
    m_gbBootloader->setLayout(layoutBootloader);

    //select file�ؼ���
    auto layoutS19Selector = new QGridLayout;
    layoutS19Selector->addWidget(m_btnChooseFile, 0, 0);
    layoutS19Selector->addWidget(m_leFileInfo, 0, 1);
    m_gbS19Selector->setLayout(layoutS19Selector);

    //diagnosis�ؼ���
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

    //����ѡ��ؼ���
    auto swFuncLayout = new QHBoxLayout;
    swFuncLayout->addWidget(m_cmbFunctionSwitch);
    swFuncLayout->addWidget(m_leRunCommand);
    m_gbSwitchFunction->setLayout(swFuncLayout);

    //�ײ��ؼ���
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

    //���ô��ں����������С
    m_btnGenerate->setFixedHeight(50);
    ui->centralWidget->setLayout(m_layoutGlobal);
}

//�����ʼ��
void FirmwareGenerator::commandsInitialization(void)
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
    cmdList.push_back({ CMD_GEN_FLASH_DRIVER, {":flash driver", ":fd"} });
    cmdList.push_back({ CMD_GEN_ERASE_EEPROM, {":erase eeprom", ":ee"} });
    cmdList.push_back({ CMD_GEN_M1_BOOT_CODE, {":m boot code", ":mbc"} });
    cmdList.push_back({ CMD_GEN_T1_BOOT_CODE, {":t boot code", ":tbc"} });
    cmdList.push_back({ CMD_GEN_S51EVFL_BOOT_CODE, {":s boot code", ":sbc"} });
    cmdList.push_back({ CMD_GEN_A13TEV_BOOT_CODE, {":a boot code", ":abc"} });
    cmdList.push_back({ CMD_DIAG_CALCULATE_KEY, {":calculate key", ":calc key", ":ck"} });
#if WIN32
    cmdList.push_back({ CMD_WINDOWS_COMMON, {"::"} });
    //�˴�Ҫ��windows��ʶ����������stringlist����λ
    cmdList.push_back({ CMD_WINDOWS_CALCULATOR, {":calc", ":calculator", ":c"} });
    cmdList.push_back({ CMD_WINDOWS_TASKMANAGER, {":taskmgr", ":tm"} });
    cmdList.push_back({ CMD_WINDOWS_PAINT, {":mspaint", ":mp"} });
#endif
}
