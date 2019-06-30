#include "canlogseparator.h"
#include "ui_canlogseparator.h"
#include <QFile>
#include <QFileDialog>
#include <QTextStream>
#include <QDateTime>
#include <QDir>
#include <QProcess>
#include <QMessageBox>
#include <QLabel>
#include <QDebug>

CanLogSeparator::CanLogSeparator(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::CanLogSeparator)
{
    ui->setupUi(this);

    initialization();
}

CanLogSeparator::~CanLogSeparator()
{
    delete ui;
}

void CanLogSeparator::initialization()
{
    //window name
    setWindowTitle(tr("CanLogSeparator"));

    //status bar
    auto labelAuthorInfo = new QLabel;
    labelAuthorInfo->setStatusTip(tr("click to view source code on github"));
    labelAuthorInfo->setOpenExternalLinks(true);
    labelAuthorInfo->setText(QString::fromLocal8Bit("<style> a {text-decoration: none} </style> <a href = https://www.github.com/bingshuizhilian/QTPROJECTS-ICM-DEVELOPMENT-TOOLBOX> contact author </a>"));
    labelAuthorInfo->show();
    ui->statusBar->addPermanentWidget(labelAuthorInfo);

    //components initialization
    m_pteOutput = new QPlainTextEdit;
    m_leSeparator = new QLineEdit;
    m_leSeparator->setToolTip(tr("input format(case insensitive): 0xhhh;0xhhhh"));
    m_leSeparator->setPlaceholderText("e.g. 0xhhh or 0xhhh;0xhhhh");
    QRegExp hexCodeRegex("^(0[xX][a-fA-F\\d]{3,4};)+$");
    auto validator = new QRegExpValidator(hexCodeRegex, m_leSeparator);
    m_leSeparator->setValidator(validator);
    m_cbLogStyle = new QComboBox;
    m_cbLogStyle->setStatusTip(tr("select a log style"));
    m_cbLogStyle->addItem(tr("time stamp"));
    m_cbLogStyle->addItem(tr("line number"));
    m_cbLogStyle->addItem(tr("time & line"));
    m_cbLogStyle->addItem(tr("none"));
    m_pbOpenLog = new QPushButton;
    m_pbOpenLog->setText(tr("open log"));
    connect(m_pbOpenLog, &m_pbOpenLog->clicked, this, &onOpenLogButtonClicked);
    m_pbShowStatistics = new QPushButton;
    m_pbShowStatistics->setText(tr("log statistics"));
    connect(m_pbShowStatistics, &m_pbShowStatistics->clicked, this, &onShowStatisticsButtonClicked);
    m_pbSaveScreen = new QPushButton;
    m_pbSaveScreen->setText(tr("save screen"));
    connect(m_pbSaveScreen, &m_pbSaveScreen->clicked, this, &onSaveScreenButtonClicked);
    m_pbClearScreen = new QPushButton;
    m_pbClearScreen->setText(tr("clear screen"));
    connect(m_pbClearScreen, &m_pbClearScreen->clicked, m_pteOutput, &m_pteOutput->clear);
    m_pbSeparateLog = new QPushButton;
    m_pbSeparateLog->setText(tr("separate log"));
    connect(m_pbSeparateLog, &m_pbSeparateLog->clicked, this, &onSeparateLogButtonClicked);

    //CAN msg filter
    m_gbFilterSettings = new QGroupBox;
    m_gbFilterSettings->setTitle(tr("filters"));
    auto m_gbFilterSettingsLayout = new QHBoxLayout;
    m_gbFilterSettingsLayout->addWidget(m_leSeparator);
    m_gbFilterSettingsLayout->addWidget(m_cbLogStyle);
    m_gbFilterSettingsLayout->addWidget(m_pbSeparateLog);
    m_gbFilterSettings->setLayout(m_gbFilterSettingsLayout);

    //main buttons
    m_gbBtns = new QGroupBox;
    m_gbBtns->setTitle(tr("operations"));
    auto m_gbBtnsLayout = new QHBoxLayout;
    m_gbBtnsLayout->addWidget(m_pbOpenLog);
    m_gbBtnsLayout->addWidget(m_pbShowStatistics);
    m_gbBtnsLayout->addWidget(m_pbSaveScreen);
    m_gbBtnsLayout->addWidget(m_pbClearScreen);
    m_gbBtns->setLayout(m_gbBtnsLayout);

    //global layout
    m_layoutGlobal = new QVBoxLayout;
    m_layoutGlobal->addWidget(m_pteOutput);
    m_layoutGlobal->addWidget(m_gbFilterSettings);
    m_layoutGlobal->addWidget(m_gbBtns);

    ui->centralWidget->setLayout(m_layoutGlobal);
    this->setMinimumSize(480, 400);
}

void CanLogSeparator::onOpenLogButtonClicked()
{
    //定义文件对话框类
    QFileDialog* fileDialog = new QFileDialog(this);
    //定义文件对话框标题
    fileDialog->setWindowTitle(tr("choose log file(s)"));
    //设置默认文件路径
    fileDialog->setDirectory(".");
    //设置文件过滤器
    fileDialog->setNameFilter(tr("*.log *.txt"));
    //设置可以选择多个文件,默认为只能选择一个文件QFileDialog::ExistingFile
    fileDialog->setFileMode(QFileDialog::ExistingFiles);
    //设置视图模式
    fileDialog->setViewMode(QFileDialog::Detail);
    //打印所有选择的文件的路径
    QStringList fileNames;

    if(fileDialog->exec())
    {
        fileNames = fileDialog->selectedFiles();

        //clear old data
        m_FileInfos.clear();
        m_OriginalLogs.clear();
        m_CanMessages.clear();
        m_Separators.clear();
        m_Statistics.clear();

        for(QString filename: fileNames)
        {
            QStringList tmp;
            tmp << filename << QFileInfo(filename).absolutePath() << QFileInfo(filename).fileName();

            m_FileInfos.insert(filename, tmp);
            m_OriginalLogs.insert(filename, QStringList());
            m_CanMessages.insert(filename, QSet<QString>());
            m_Separators.insert(filename, QStringList());
            m_Statistics.insert(filename, QStringList());
        }
    }

    if(fileNames.isEmpty())
        return;

    if(m_FileInfos.isEmpty())
        return;

    for(auto tmp:m_FileInfos)
        qDebug()<<tmp<<endl;

    //clear output
    m_pteOutput->clear();

    for(QString fileNameIter: m_FileInfos.keys())
    {
        //get original logs
        QFile logFile(fileNameIter);
        if(!logFile.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            QMessageBox::warning(this, "Warnning", "Cannot open " + fileNameIter, QMessageBox::Yes);
            return;
        }

        QTextStream logFileIn(&logFile);
        while(!logFileIn.atEnd())
        {
            QString readStr = logFileIn.readLine();

            //busmaster log format
            if(!readStr.isEmpty() && !readStr.startsWith('*') && !readStr.startsWith('"'))
            {
                m_OriginalLogs[fileNameIter].push_back(readStr);

                if(readStr.contains("0x", Qt::CaseInsensitive))
                {
                    int index1 = 0, index2 = 0;
                    QString tmpStr;

                    index1 = readStr.indexOf("0x", Qt::CaseInsensitive);

                    if(-1 != index1)
                        index2 = readStr.indexOf(' ', index1, Qt::CaseInsensitive);

                    if(-1 != index2)
                        tmpStr = readStr.mid(index1, index2 - index1);

                    if(!tmpStr.isEmpty())
                        m_CanMessages[fileNameIter].insert(tmpStr.toLower());
                }
            }
        }
        logFile.close();

        //statistics info output
        m_pteOutput->appendPlainText("####in log file: " + m_FileInfos[fileNameIter].at(ABSOLUTE_FILE_PATH) + "####");

        m_Statistics[fileNameIter] << "***all detected can message list***";
        foreach(const QString &value, m_CanMessages[fileNameIter])
            m_Statistics[fileNameIter] << value;

        m_Statistics[fileNameIter] << "***statistics infos***";
        m_Statistics[fileNameIter] << "Total effective lines: " + QString::number(m_OriginalLogs[fileNameIter].size());
        m_Statistics[fileNameIter] << "Total different can messages: " + QString::number(m_CanMessages[fileNameIter].size());

        if(fileNameIter != m_FileInfos.keys().last())
            m_Statistics[fileNameIter] << "\n";

        foreach(const QString &value, m_Statistics[fileNameIter])
            m_pteOutput->appendPlainText(value);
    }

    m_pbOpenLog->setStatusTip("amount of current loaded files: " + QString::number(m_FileInfos.size()));
}

void CanLogSeparator::onShowStatisticsButtonClicked()
{
    m_pteOutput->clear();

    if(!m_FileInfos.isEmpty() && !m_Statistics.isEmpty())
    {
        for(QString fileNameIter: m_FileInfos.keys())
        {
            m_pteOutput->appendPlainText("####in log file: " + m_FileInfos[fileNameIter].at(ABSOLUTE_FILE_PATH) + "####");

            foreach(const QString &value, m_Statistics[fileNameIter])
                m_pteOutput->appendPlainText(value);
        }
    }
}

void CanLogSeparator::onSaveScreenButtonClicked()
{
    if(m_pteOutput->toPlainText().isEmpty() || m_FileInfos.isEmpty())
        return;

    //save screen text
    QString tmpFileName = m_FileInfos.first().at(FILE_NAME);
    QString tmpFileClassId = tmpFileName.right(4);
    QString timeInfo = QDateTime::currentDateTime().toString("yyyy-MM-dd_HH-mm-ss");
    tmpFileName = tmpFileName.left(tmpFileName.size() - 4);
    if(1 == m_FileInfos.size())
        tmpFileName += "_SeparatedLog_" + timeInfo + tmpFileClassId;
    else
        tmpFileName += "_With_" + QString::number(m_FileInfos.size() - 1) + "_OtherFilesSeparatedLogs_" + timeInfo + tmpFileClassId;

    qDebug()<<tmpFileName<<endl;

    QString folderName = "/separatedLogs/";
    QString dirPath = m_FileInfos.first().at(ABSOLUTE_PATH) + folderName;
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

    if(1 == m_FileInfos.size())
    {
        out << "original log file:\n" + m_FileInfos.first().at(ABSOLUTE_FILE_PATH) + '\n';
    }
    else
    {
        out << "original log files:\n";

        for(QString fileNameIter: m_FileInfos.keys())
        {
            out << m_FileInfos[fileNameIter].at(ABSOLUTE_FILE_PATH) + '\n';
        }
    }

    out << "\n\n" + m_pteOutput->toPlainText();

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

void CanLogSeparator::onSeparateLogButtonClicked()
{
    QString separator = m_leSeparator->text();
    if(separator.isEmpty() || m_CanMessages.isEmpty())
        return;

    QStringList tmpSeparators;
    tmpSeparators = separator.split(';', QString::SkipEmptyParts, Qt::CaseInsensitive);

    for(QString fileNameIter: m_FileInfos.keys())
    {
        m_Separators[fileNameIter].clear();
        foreach(const QString &value, tmpSeparators)
        {
            //can id format: 0x3af 0x5edd
            if(value.size() != 5 && value.size() != 6)
            {
                QMessageBox::warning(this, "Warnning", "please check input", QMessageBox::Yes);
                return;
            }

            if(!m_CanMessages[fileNameIter].contains(value.toLower()))
            {
                QMessageBox::information(this, "Warnning", "none " + value + " record was founded in "
                                         + m_FileInfos[fileNameIter].at(ABSOLUTE_FILE_PATH), QMessageBox::Yes);
                continue;
            }

            m_Separators[fileNameIter] << value.toLower();
        }
    }

    if(m_Separators.isEmpty())
        return;

    m_pteOutput->clear();

    QString bmStr;
    switch(m_cbLogStyle->currentIndex())
    {
    case 0:
        bmStr = busmaster;
        break;
    case 1:
        bmStr = busmaster.right(busmaster.size() - busmaster.indexOf("<CAN ID>", Qt::CaseInsensitive));
        bmStr += "<Line Number>";
        break;
    case 2:
        bmStr = busmaster + "<Line Number>";
        break;
    case 3:
        bmStr = busmaster.right(busmaster.size() - busmaster.indexOf("<CAN ID>", Qt::CaseInsensitive));
        break;
    default:
        break;
    }

    for(QString fileNameIter: m_FileInfos.keys())
    {
        m_pteOutput->appendPlainText("--->" + m_FileInfos[fileNameIter].at(ABSOLUTE_FILE_PATH) + "<---");
        m_pteOutput->appendPlainText(bmStr);

        unsigned int lineNumber = 1;
        foreach(const QString &value1, m_OriginalLogs[fileNameIter])
        {
            foreach(const QString &value2, m_Separators[fileNameIter])
            {
                if(value1.contains(value2, Qt::CaseInsensitive))
                {
                    QString tmpStr;
                    switch(m_cbLogStyle->currentIndex())
                    {
                    case 0:
                        tmpStr = value1;
                        break;
                    case 1:
                        tmpStr = value1.right(value1.size() - value1.indexOf("0x", Qt::CaseInsensitive));
                        tmpStr += "  (line: " + QString::number(lineNumber++) + ")";
                        break;
                    case 2:
                        tmpStr = value1 + "  (line: " + QString::number(lineNumber++) + ")";
                        break;
                    case 3:
                        tmpStr = value1.right(value1.size() - value1.indexOf("0x", Qt::CaseInsensitive));
                        break;
                    default:
                        tmpStr = value1;
                        break;
                    }

                    m_pteOutput->appendPlainText(tmpStr);
                }
            }
        }

        if(fileNameIter != m_FileInfos.keys().last())
            m_pteOutput->appendPlainText("\n\n");
    }

    QTextCursor cursor = m_pteOutput->textCursor();
    cursor.movePosition(QTextCursor::Start);
    m_pteOutput->setTextCursor(cursor);
}
