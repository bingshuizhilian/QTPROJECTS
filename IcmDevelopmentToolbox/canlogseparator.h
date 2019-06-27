#ifndef CANLOGSEPARATOR_H
#define CANLOGSEPARATOR_H

#include <QMainWindow>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QComboBox>
#include <QLayout>
#include <QGroupBox>
#include <QStringList>
#include <QSet>
#include <QLineEdit>
#include <QMap>

namespace Ui {
class CanLogSeparator;
}

class CanLogSeparator : public QMainWindow
{
    Q_OBJECT

public:
    explicit CanLogSeparator(QWidget *parent = 0);
    ~CanLogSeparator();

private:
    Ui::CanLogSeparator *ui;

private:
    enum FileInfoType
    {
        ABSOLUTE_FILE_PATH,
        ABSOLUTE_PATH,
        FILE_NAME
    };

    const QString busmaster = "<Time><Tx/Rx><Channel><CAN ID><Type><DLC><DataBytes>";

private:
    QMap<QString, QStringList> m_FileInfos;
    QMap<QString, QStringList> m_OriginalLogs;
    QMap<QString, QSet<QString>> m_CanMessages;
    QMap<QString, QStringList> m_Separators;
    QMap<QString, QStringList> m_Statistics;

private:
    QPlainTextEdit *m_pteOutput;
    QPushButton *m_pbOpenLog;
    QPushButton *m_pbShowStatistics;
    QPushButton *m_pbSaveScreen;
    QPushButton *m_pbClearScreen;
    QLineEdit *m_leSeparator;
    QComboBox *m_cbLogStyle;
    QPushButton *m_pbSeparateLog;

private:
    QVBoxLayout* m_layoutGlobal;
    QGroupBox* m_gbBtns;
    QGroupBox* m_gbFilterSettings;

private:
    void initialization();

private slots:
    void onOpenLogButtonClicked();
    void onShowStatisticsButtonClicked();
    void onSaveScreenButtonClicked();
    void onSeparateLogButtonClicked();
};

#endif // CANLOGSEPARATOR_H
