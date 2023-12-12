#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QSerialPortInfo>
#include <QMessageBox>
#include <QFileDialog>
#include <QScrollBar>
#include <QTimer>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_autoSendTimer(new QTimer(this))
{
    ui->setupUi(this);

    //主界面
    this->setWindowTitle("QSTCFlash");
    this->centralWidget()->setLayout(ui->hLayout);
    ui->tab_download->setLayout(ui->tabDownloadLayout);
    ui->tab_helper->setLayout(ui->helperTabLayout);

    ui->combo_models->addItems(NS_McuData::protocolList);
    ui->combo_lowBaud->addItems(NS_McuData::baudrateList);
    ui->combo_highBaud->addItems(NS_McuData::baudrateList);
    ui->combo_helperBaud->addItems(NS_McuData::baudrateList);
    ui->combo_lowBaud->setCurrentText("2400");
    ui->combo_highBaud->setCurrentText("115200");
    ui->combo_helperBaud->setCurrentText("57600");
    m_downloadPort.setBaudRate(2400);
    m_downloadPort.setDataBits(QSerialPort::Data8);
    m_downloadPort.setStopBits(QSerialPort::OneStop);
    this->flushPorts();

    ui->combo_dataBits->setItemData(0, QSerialPort::Data5);
    ui->combo_dataBits->setItemData(1, QSerialPort::Data6);
    ui->combo_dataBits->setItemData(2, QSerialPort::Data7);
    ui->combo_dataBits->setItemData(3, QSerialPort::Data8);
    ui->combo_dataBits->setCurrentIndex(3);

    m_autoSendTimer->setInterval(ui->spin_interval->value());

    //信号
    connect(&m_helperPort, SIGNAL(readyRead()), this, SLOT(onHelperRecieved()));
    connect(ui->btn_flush, SIGNAL(clicked()), this, SLOT(flushPorts()));
    connect(ui->combo_helperBaud, SIGNAL(currentTextChanged(const QString&)), this, SLOT(onHelperBaudChanged(const QString&)));
    connect(ui->combo_parity, SIGNAL(currentIndexChanged(int)), this, SLOT(onHelperParityChanged(int)));
    connect(ui->combo_stopBits, SIGNAL(currentIndexChanged(int)), this, SLOT(onHelperStopBitsChanged(int)));
    connect(ui->combo_dataBits, SIGNAL(currentIndexChanged(int)), this, SLOT(onHelperDataBitsChanged(int)));
    connect(ui->btn_download, SIGNAL(clicked()), this, SLOT(program()));
    connect(ui->check_autoSend, SIGNAL(stateChanged(int)), this, SLOT(onAutoSendStateChanged(int)));
    connect(ui->spin_interval, SIGNAL(valueChanged(int)), this, SLOT(onAutoSendIntervalChanged(int)));
    connect(m_autoSendTimer, SIGNAL(timeout()), this, SLOT(onAutoSendTimeout()));
    connect(ui->radio_recieveHex, SIGNAL(toggled(bool)), this, SLOT(onRecieveHexToggled(bool)));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::flushPorts()
{
    QList<QSerialPortInfo> infoList = QSerialPortInfo::availablePorts();

    QStringList portList;

    for (int i = 0; i < infoList.size(); ++i) {
        portList << infoList.at(i).portName();
    }

    ui->combo_downloadPorts->clear();
    ui->combo_downloadPorts->addItems(portList);
    ui->combo_helperPorts->clear();
    ui->combo_helperPorts->addItems(portList);
}

void MainWindow::on_btn_selectFile_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("选择文件"), "" == m_file? QDir::homePath() : m_file, tr("Hex文件(*.hex *.ihx);;全部文件(*)"));
    if ("" == fileName) {
        return;
    }

    m_file = fileName;
    ui->statusbar->showMessage(tr("已选择文件“%1”").arg(m_file));
}

void MainWindow::on_btn_detect_clicked()
{
    if (tr("停止") == ui->btn_detect->text()) {
        m_stopped = true;
        ui->btn_detect->setText(tr("检测"));
        return;
    }

    if (m_helperPort.portName() == ui->combo_downloadPorts->currentText() && m_helperPort.isOpen()) {
        this->on_btn_openOrClose_clicked();
    }

    m_downloadPort.setParity(QSerialPort::NoParity);
    m_downloadPort.setBaudRate(ui->combo_lowBaud->currentText().toUInt());
    m_downloadPort.setPortName(ui->combo_downloadPorts->currentText());
    m_mcuInfo.info.clear();
    m_mcuInfo.checkMode = 0;

    if (!m_downloadPort.open(QIODevice::ReadWrite)) {
        QMessageBox::critical(this, tr("错误"), errorMessage(m_downloadPort.error()), tr("确定"));
        return;
    }

    ui->outputText->clear();
    ui->tabWidget->setCurrentIndex(0);
    ui->outputText->appendPlainText(tr("检测目标..."));
    ui->btn_detect->setText(tr("停止"));
    m_stopped = false;
    bool succeed = this->detect();
    ui->outputText->insertPlainText(tr(succeed? "完成" : "失败"));

    if (NS_McuData::Protocol_None == m_mcuInfo.protocol) {
        if (m_mcuInfo.info.length() > 4) {
            ui->outputText->appendPlainText(tr("未知型号：%1 %2").arg((uchar)m_mcuInfo.info.at(3), 2, 16, QLatin1Char('0')).arg((uchar)m_mcuInfo.info.at(4), 2, 16, QLatin1Char('0')).toUpper());
        }
    }else if (succeed) {
        this->printInfo();
    }

    m_downloadPort.close();
    ui->btn_detect->setText(tr("检测"));
}

QString MainWindow::errorMessage(int code)
{
    QString text;

    switch (code) {
    case QSerialPort::DeviceNotFoundError: text = tr("设备不存在"); break;
    case QSerialPort::PermissionError: text = tr("没有权限或端口忙"); break;
    case QSerialPort::OpenError: text = tr("打开设备出错"); break;
    case QSerialPort::NotOpenError: text = tr("设备未打开"); break;
    case QSerialPort::WriteError: text = tr("写入设备出错"); break;
    case QSerialPort::ReadError: text = tr("读取设备出错"); break;
    case QSerialPort::ResourceError: text = tr("数据源错误（设备异常或已被移除）"); break;
    case QSerialPort::UnsupportedOperationError: text = tr("不支持的操作"); break;
    case QSerialPort::TimeoutError: text = tr("超时错误"); break;
    case QSerialPort::UnknownError: text = tr("未知错误"); break;
    }

    return text;
}


void MainWindow::on_btn_openOrClose_clicked()
{
    if (m_helperPort.isOpen()) {
        m_helperPort.close();
    }

    if (tr("关闭串口") == ui->btn_openOrClose->text()) { //在打开状态
        ui->btn_openOrClose->setText(tr("打开串口"));
        ui->btn_send->setEnabled(false);
        ui->check_autoSend->setEnabled(false);
        if (ui->check_autoSend->isChecked()) {
            m_autoSendTimer->stop();
            ui->check_autoSend->setChecked(false);
        }
        ui->spin_interval->setEnabled(false);
        return;
    }

    m_helperPort.setPortName(ui->combo_helperPorts->currentText());
    m_helperPort.setBaudRate(ui->combo_helperBaud->currentText().toUInt());
    this->onHelperDataBitsChanged(ui->combo_dataBits->currentIndex());
    this->onHelperParityChanged(ui->combo_parity->currentIndex());
    this->onHelperStopBitsChanged(ui->combo_stopBits->currentIndex());

    if (!m_helperPort.open(QIODevice::ReadWrite)) {
        QMessageBox::critical(this, tr("错误"), errorMessage(m_helperPort.error()), tr("确定"));
    }else {
        ui->btn_send->setEnabled(true);
        ui->check_autoSend->setEnabled(true);
        ui->spin_interval->setEnabled(true);
        ui->btn_openOrClose->setText(tr("关闭串口"));
    }
}

void MainWindow::on_btn_send_clicked()
{
    QString text = ui->sendText->toPlainText();
    if (text.isEmpty()) {return;}

    QByteArray sendData;
    int i = 0, size = text.size();
    bool next;
    for (; i < size;) {
        if (ui->radio_sendHex->isChecked()) {
            if ((text.at(i) >= '0' && text.at(i) <='9')
                || (text.at(i) >= 'a' && text.at(i) <= 'f')
                || (text.at(i) >= 'A' && text.at(i) <= 'F'))
            {
                next = (++i < size) && ((text.at(i) >= '0' && text.at(i) <='9')
                                        || (text.at(i) >= 'a' && text.at(i) <= 'f')
                                        || (text.at(i) >= 'A' && text.at(i) <= 'F'));
                sendData += text.midRef(i - 1, 1 + next).toUInt(nullptr, 16);
            }
            ++i;
        }else {
            sendData += text.at(i++).toLatin1();
        }
    }

    m_helperPort.write(sendData);
}

void MainWindow::onHelperRecieved()
{
    QByteArray data = m_helperPort.readAll();
    QString text = ui->recievedText->toPlainText();

    foreach (const unsigned char &ch, data) {
        if (ui->radio_recieveHex->isChecked()) {
            text += QString("%1 ").arg(ch, 2, 16, QLatin1Char('0')).toUpper();
        }else {
            text += ch;
        }
    }
    
    ui->recievedText->setPlainText(text);
    m_recievedData += data;
    if (ui->recievedText->verticalScrollBar()->isVisible()) {
        ui->recievedText->verticalScrollBar()->setValue(ui->recievedText->verticalScrollBar()->maximum());
    }
}


void MainWindow::on_btn_clearRecieve_clicked()
{
    ui->recievedText->clear();
    m_recievedData.clear();
}


void MainWindow::on_btn_clearSend_clicked()
{
    ui->sendText->clear();
}


void MainWindow::onAutoSendStateChanged(int state)
{
    if (Qt::Checked == state) {
        m_autoSendTimer->start();
    }else {
        m_autoSendTimer->stop();
    }
}


void MainWindow::onAutoSendIntervalChanged(int n)
{
    m_autoSendTimer->setInterval(n);
}

void MainWindow::onAutoSendTimeout()
{
    this->on_btn_send_clicked();
}


void MainWindow::onHelperBaudChanged(const QString &baud)
{
    m_helperPort.setBaudRate(baud.toUInt());
}

void MainWindow::onHelperParityChanged(int index)
{
    switch (index) {
    case 1:
        m_helperPort.setParity(QSerialPort::EvenParity); break;
    case 2:
        m_helperPort.setParity(QSerialPort::OddParity); break;
    default:
        m_helperPort.setParity(QSerialPort::NoParity);
    }
}

void MainWindow::onHelperStopBitsChanged(int index)
{
    if (index) {
        m_helperPort.setStopBits(QSerialPort::TwoStop);
    }else {
        m_helperPort.setStopBits(QSerialPort::OneStop);
    }
}

void MainWindow::onHelperDataBitsChanged(int index)
{
    m_helperPort.setDataBits((QSerialPort::DataBits) ui->combo_dataBits->itemData(index, Qt::UserRole).toInt());
}


void MainWindow::onRecieveHexToggled(bool checked)
{
    QString text;
    int scroll = ui->recievedText->verticalScrollBar()->value();
    if (checked) {
        foreach (const unsigned char &ch, m_recievedData) {
            text += QString("%1 ").arg(ch, 2, 16, QLatin1Char('0')).toUpper();
        }
    }else {
        foreach (const unsigned char &ch, m_recievedData) {
            text += ch;
        }
    }

    ui->recievedText->setPlainText(text);

    if (ui->recievedText->verticalScrollBar()->isVisible()) {
        ui->recievedText->verticalScrollBar()->setValue(scroll);
    }
}
