#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "mcudata.h"

#include <QMainWindow>
#include <QSerialPort>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();



private slots:
    void flushPorts();
    void on_btn_selectFile_clicked();
    void on_btn_clearRecieve_clicked();
    void on_btn_clearSend_clicked();
    void onAutoSendStateChanged(int state);
    void onAutoSendIntervalChanged(int n);
    void onAutoSendTimeout();

    void on_btn_openOrClose_clicked();
    void on_btn_send_clicked();
    void onHelperBaudChanged(const QString &boud);
    void onHelperParityChanged(int index);
    void onHelperStopBitsChanged(int index);
    void onHelperDataBitsChanged(int index);

    void onHelperRecieved();

    void on_btn_detect_clicked();

    void program(); //stcflash

    void onRecieveHexToggled(bool checked);

private:
    QString errorMessage(int);

    //stcflash
    QByteArray hex2bin(const QString &file, bool *ok);
    bool detect();
    void printInfo();
    bool handshake();
    QByteArray recieve(int timeout_ms = 1000, const QByteArray &head = "\x46\xB9\x68");
    void send(uchar cmd, QByteArray data);
    bool erase();
    bool flash(const QByteArray &code);
    bool setOptions(const QByteArray &info);
    void terminate();
    bool unkownPacket_1();
    bool unkownPacket_2();
    bool unkownPacket_3();


    Ui::MainWindow *ui;

    struct NS_McuData::McuInfo m_mcuInfo;
    QSerialPort m_downloadPort; //下载端口
    QSerialPort m_helperPort; //串口助手端口
    QString m_file;
    QTimer *m_autoSendTimer;
    QByteArray m_recievedData;
    //bool m_autoSend;
    bool m_stopped;
};
#endif // MAINWINDOW_H
