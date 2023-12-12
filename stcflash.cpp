#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMessageBox>
#include <QTime>
#include <QElapsedTimer>
#include <QFileInfo>


QByteArray MainWindow::hex2bin(const QString &fileName, bool *ok)
{
    QByteArray data;
    QFile file(fileName);
    uint err = 0, i = 0, j;
    const QStringList errMsg = {
        tr("无法打开文件“%1”"),
        tr("文件“%2”第 %1 行，数据错误"),
        tr("文件“%2”第 %1 行，不支持的数据类型")
    };

    if (file.open(QIODevice::ReadOnly)) {
        QStringList list = QString::fromUtf8(file.readAll()).replace("\r", "").split("\n");
        QByteArray buf;
        uint base = 0, //基址
             addr,
             len,
             size = list.size();
        uchar checkSum;

        for (; i < size; ++i) {
            if (!list.at(i).length()) {
                continue;
            }

            if (list.at(i).at(0) != ':') {
                err = 2;
                break;
            }

            buf = QByteArray::fromHex(list.at(i).midRef(1).toLatin1());
            checkSum = 0;
            len = buf.length();

            for (j = 0; j < len; ++j) {
                checkSum += (uchar)buf[j];
            }

            if (checkSum) {
                err = 2;
                break;
            }

            switch (buf.at(3)) {
            case 0:
                addr = base + ((uchar)buf.at(1) << 8) + (uchar)buf.at(2);
                data.append(addr + (uchar)buf.at(0) - data.length(), 0xFF);
                memcpy(&data.data()[addr], &buf.data()[4], (uchar)buf.at(0));
                break;
            case 1:
                if (buf.at(0)) {
                    err = 2;
                }else {
                    i = size; //结束
                }
                break;
            case 2:
                if (buf.at(0) != 2) {
                    err = 2;
                }
                base = (((uchar)buf.at(4) << 8) + (uchar)buf.at(5)) << 4;
                break;
            case 4:
                if (buf.at(0) != 2) {
                    err = 2;
                }
                base = (((uchar)buf.at(4) << 8) + (uchar)buf.at(5)) << 16;
                break;
            default:
                err = 3;
            }

            if (err) {
                break;
            }
        }
    }else {
        err = 1;
    }

    if (err) {
        QMessageBox::critical(this, tr("错误"), errMsg.at(err - 1).arg(i + 1).arg(fileName), tr("确定"));
    }

    *ok = !err;
    return data;
}

bool MainWindow::detect()
{
    QByteArray data;
    QElapsedTimer timer;
    int timeout = 10;
    bool succeed = false;

    m_downloadPort.write("\x7F");
    m_downloadPort.waitForBytesWritten(20);
    timer.start();
    while (!m_stopped && timer.elapsed() < 15000) { //探测 15s 时间
        if (timer.elapsed() > timeout) { //间隔 10 毫秒发送一次
            m_downloadPort.write("\x7F");
            m_downloadPort.waitForBytesWritten(20);
            timeout = timer.elapsed() + 10;
        }

        data = m_downloadPort.read(1);

        if ("\x68"== data) {
            timeout = timer.elapsed() + 30; //延时 30ms，读取包长度
            while (timer.elapsed() < timeout) {
                QApplication::processEvents();
            }
            data += m_downloadPort.read(3 - data.length());
            if (data.length() >= 3) {
                succeed = true;
                break;
            }
        }

        QApplication::processEvents();
    }

    if (succeed) {
        uint chksum = 0;
        int len = ((uchar)data.at(1) << 8) + (uchar)data.at(2);
        if (len > 64 || len < 25) {
            QMessageBox::critical(this, tr("错误"), tr("不正确的包长度"), tr("确定"));
            return false;
        }

        timer.start();
        while (data.length() < len && timer.elapsed() < 500) {
            data += m_downloadPort.read(len - data.length());
            QApplication::processEvents();
        }

        if (data.length() < len) {
            QMessageBox::critical(this, tr("错误"), tr("等待接收数据超时"), tr("确定"));
            return false;
        }


        if ((uchar)data.at(len - 1) != PACKAGE_END) {
            QMessageBox::critical(this, tr("错误"), tr("包结束标识丢失"), tr("确定"));
            return false;
        }

        for (int i = len - 2 - m_mcuInfo.checkMode; i >= 0; --i) {
            chksum += (uchar)data.at(i);
        }

        if (m_mcuInfo.checkMode > 0 && (chksum & 0xFF) != (uchar)data.at(len - 2)) {
            QMessageBox::critical(this, tr("错误"), tr("校验和[0]错误"), tr("确定"));
            return false;
        }

        if (m_mcuInfo.checkMode > 1 && ((chksum >> 8) & 0xFF) != (uchar)data.at(len - 3)) {
            QMessageBox::critical(this, tr("错误"), tr("校验和[1]错误"), tr("确定"));
            return false;
        }

        m_mcuInfo.info = data.mid(20, len - 20 - 1 - m_mcuInfo.checkMode);
        m_mcuInfo.version = QString("%1.%2%3").arg((uchar)data.at(20) >> 4).arg((uchar)data.at(20) & 0x0F).arg(data.at(21));
        int key = NS_McuData::mcuModelMapKey((uchar)data.at(23), (uchar)data.at(24));

        if (key > 0) {
            m_mcuInfo.romSize_KB = ((uchar)data.at(24) - NS_McuData::mcuModelMap.value(key).base) * NS_McuData::mcuModelMap.value(key).romRatio;
        }

        m_mcuInfo.protocol = NS_McuData::protocol((uchar)data.at(23));

        if (NS_McuData::Protocol_8H4K == m_mcuInfo.protocol || NS_McuData::Protocol_8 == m_mcuInfo.protocol) {
            if (len < 43) {return false;}
            m_mcuInfo.oscillator_Hz = ((uchar)data.at(4) << 24) + ((uchar)data.at(5) << 16) + ((uchar)data.at(6) << 8);
            m_mcuInfo.innerVref_mV = ((uchar)data.at(38) << 8) + (uchar)data.at(39);
            m_mcuInfo.wakeupOscillator_Hz = ((uchar)data.at(26) << 8) + (uchar)data.at(27);
            m_mcuInfo.testDate = tr("20%1年%2月%3日").arg((uchar)data.at(40), 2, 16, QChar('0')).arg((uchar)data.at(41), 2, 16, QChar('0')).arg((uchar)data.at(42), 2, 16, QChar('0'));
            m_mcuInfo.version = QString("%1.%2.%3%4")
                                    .arg((uchar)data.at(20) >> 4)
                                    .arg((uchar)data.at(20) & 0x0F)
                                    .arg((uchar)data.at(25))
                                    .arg(data.at(21));

            if ((data.at(14) & 0x03) == 0x03) {
                m_mcuInfo.lowVoltage_V = 2.0;
            }else {
                m_mcuInfo.lowVoltage_V = (3 - (data.at(14) & 0x03)) * 0.3 + 2.1;
            }
        }else if (NS_McuData::Protocol_15 == m_mcuInfo.protocol) {
            if (len < 48) {return false;}
            m_mcuInfo.oscillator_Hz = ((uchar)data.at(11) << 24) + ((uchar)data.at(12) << 16) + ((uchar)data.at(13) << 8);
            m_mcuInfo.innerVref_mV = ((uchar)data.at(38) << 8) + (uchar)data.at(39);
            m_mcuInfo.wakeupOscillator_Hz = ((uchar)data.at(4) << 8) + (uchar)data.at(5);
            m_mcuInfo.testDate = tr("20%1年%2月%3日").arg((uchar)data.at(45), 2, 16, QChar('0')).arg((uchar)data.at(46), 2, 16, QChar('0')).arg((uchar)data.at(47), 2, 16, QChar('0'));
            m_mcuInfo.version = QString("%1.%2.%3%4")
                                   .arg((uchar)data.at(20) >> 4)
                                   .arg((uchar)data.at(20) & 0x0F)
                                   .arg((uchar)data.at(25))
                                   .arg(data.at(21));
        }else {
            m_mcuInfo.oscillator_Hz = 0;
            for (int i = 4; i < 20; i += 2) {
                m_mcuInfo.oscillator_Hz += ((uchar)data.at(i) << 8) + (uchar)data.at(i + 1);
            }

            m_mcuInfo.oscillator_Hz = m_mcuInfo.oscillator_Hz / 8.0 * m_downloadPort.baudRate() / 580974 * 1000000 ;
        }

        if (NS_McuData::Protocol_8H4K == m_mcuInfo.protocol
            || NS_McuData::Protocol_8 == m_mcuInfo.protocol
            || NS_McuData::Protocol_15 == m_mcuInfo.protocol
            || NS_McuData::Protocol_12C5A == m_mcuInfo.protocol
            || NS_McuData::Protocol_12C52 == m_mcuInfo.protocol)
        {
            m_mcuInfo.checkMode = 2;
            m_downloadPort.setParity(QSerialPort::EvenParity);
        }else {
            m_mcuInfo.checkMode = 1;
            m_downloadPort.setParity(QSerialPort::NoParity);
        }
    }

    return succeed;
}

void MainWindow::printInfo()
{
    ui->outputText->appendPlainText(tr("系统时钟频率: %1MHz").arg(m_mcuInfo.oscillator_Hz / 1000000.0, 0, 'f', 3));

    if (NS_McuData::Protocol_8H4K == m_mcuInfo.protocol || NS_McuData::Protocol_8 == m_mcuInfo.protocol) {
        ui->outputText->appendPlainText(tr("掉电唤醒定时器频率: %1KHz").arg(m_mcuInfo.wakeupOscillator_Hz / 1000.0, 0, 'f', 3));
        ui->outputText->appendPlainText(tr("内部参考电压: %1 mV").arg(m_mcuInfo.innerVref_mV));
        ui->outputText->appendPlainText(tr("低压检测电压: %1 V").arg(m_mcuInfo.lowVoltage_V, 0, 'f', 1));
        ui->outputText->appendPlainText(tr("内部安排测试时间: %1").arg(m_mcuInfo.testDate));
    }else if (NS_McuData::Protocol_15 == m_mcuInfo.protocol) {
        ui->outputText->appendPlainText(tr("掉电唤醒定时器频率: %1KHz").arg(m_mcuInfo.wakeupOscillator_Hz / 1000.0, 0, 'f', 3));
        ui->outputText->appendPlainText(tr("内部参考电压: %1 mV").arg(m_mcuInfo.innerVref_mV));
        ui->outputText->appendPlainText(tr("内部安排测试时间: %1").arg(m_mcuInfo.testDate));
    }

    QString name = NS_McuData::mcuModelMap.value(NS_McuData::mcuModelMapKey((uchar)m_mcuInfo.info.at(3), (uchar)m_mcuInfo.info.at(4))).name;
    if (0xF0 == (uchar)m_mcuInfo.info.at(3) || 0xF1 == (uchar)m_mcuInfo.info.at(3)) {//model[0]
        name = name.arg((uchar) m_mcuInfo.info.at(4) - NS_McuData::mcuModelMap.value(NS_McuData::mcuModelMapKey((uchar)m_mcuInfo.info.at(3), (uchar)m_mcuInfo.info.at(4))).base);
    }else {
        name = name.arg(m_mcuInfo.romSize_KB);
    }
    if ("IAP" != name.left(3)) {
        name = "STC" + name;
    }

    ui->outputText->appendPlainText(tr("单片机型号: %1").arg(name));
    ui->outputText->appendPlainText(tr("固件版本号: %1").arg(m_mcuInfo.version));
    if (m_mcuInfo.romSize_KB)
        ui->outputText->appendPlainText(tr("程序空间: %1KB").arg(m_mcuInfo.romSize_KB));
}

QByteArray MainWindow::recieve(int timeout, const QByteArray &head)
{
    QByteArray data;
    QElapsedTimer timer;
    int len = head.length(), delay;
    bool found = false;

    timer.start();
    while (!m_stopped && timer.elapsed() < timeout) {
        data += m_downloadPort.read(len - data.length());

        if (head == data) {
            delay = timer.elapsed() + 20; //延时 20ms，读取包长度
            while (timer.elapsed() < delay) {
                QApplication::processEvents();
            }
            data += m_downloadPort.read(len + 2 - data.length());
            if (data.length() >= len + 2) {
                found = true;
                break;
            }
        }else if (data.length() >= len){
            data.remove(0, 1);
        }

        QApplication::processEvents();
    }

    if (found) {
        uint chksum = 0;
        int n = ((uchar)data.at(len) << 8) + (uchar)data.at(len + 1);
        if (n > 64) {
            QMessageBox::critical(this, tr("错误"), tr("不正确的包长度"), tr("确定"));
            found = false;
            goto final;
        }

        len += 2 + n - 3;
        timer.start();
        while (data.length() < len && timer.elapsed() < 500) {
            data += m_downloadPort.read(len - data.length());
            QApplication::processEvents();
        }

        if (data.at(data.length() - 1) != PACKAGE_END) {
            QMessageBox::critical(this, tr("错误"), tr("包结束标识丢失"), tr("确定"));
            found = false;
            goto final;
        }

        for (int i = data.length() - 2 - m_mcuInfo.checkMode; i > 1; --i) {
            chksum += (uchar)data.at(i);
        }

        if (m_mcuInfo.checkMode > 0 && (chksum & 0xFF) != (uchar)data.at(data.length() - 2)) {
            QMessageBox::critical(this, tr("错误"), tr("校验和[0]错误"), tr("确定"));
            found = false;
            goto final;
        }

        if (m_mcuInfo.checkMode > 1 && ((chksum >> 8) & 0xFF) != (uchar)data.at(data.length() - 3)) {
            QMessageBox::critical(this, tr("错误"), tr("校验和[1]错误"), tr("确定"));
            found = false;
            goto final;
        }
    }else {
        QMessageBox::critical(this, tr("错误"), tr("等待接收数据超时"), tr("确定"));
    }

final:
    if (!found) {
        data.clear();
    }

    return data;
}

void MainWindow::send(uchar cmd, QByteArray data)
{
    QByteArray buf = "\x46\xB9\x6A";
    uint chksum = 0, len = 1 + 2 + 1 + data.length() + m_mcuInfo.checkMode + 1;
    buf.append(len >> 8);
    buf.append(len);
    buf.append(cmd);
    buf += data;

    for (len = buf.length() - 1; len >= 2; --len) {
        chksum += (uchar)buf.at(len);
    }

    if (m_mcuInfo.checkMode > 1) {
        buf.append(chksum >> 8);
    }

    buf.append(chksum);
    buf.append(PACKAGE_END);
    m_downloadPort.write(buf);
}

bool MainWindow::handshake()
{
    uint timer1Value, baud0 = m_downloadPort.baudRate(),
            baud = ui->combo_highBaud->currentText().toUInt();
    uchar ocValue, ocValue2;
    QByteArray data, recievedData;
    QElapsedTimer timer;

    if (NS_McuData::Protocol_8H4K == m_mcuInfo.protocol) {
        //第一个握手包
        data.resize(17);
        memcpy(data.data(), "\x08\x00\x00\xFF\x00\x00\x10\xFF\x10\x00\x20\xFF\x20\x00\x30\xFF\x30", 17);
        this->send(0x00, data);
        m_downloadPort.waitForBytesWritten(200);
        m_downloadPort.write("\xFE");
        int timeout = 10, len;
        timer.start();
        while (recievedData.length() < 5 && timer.elapsed() < 1000) {
            m_downloadPort.write("\xFE");
            while (timer.elapsed() < timeout) {
                QApplication::processEvents();
            }
            recievedData += m_downloadPort.read(5 - recievedData.length());
            timeout += 10;
        }

        if(!recievedData.length()) {return false;}
        timer.start();
        while (timer.elapsed() < 50) {
            QApplication::processEvents();
        }

        if (recievedData.length() >= 5) {
            len = 2 + ((uchar)recievedData.at(3) << 8) + (uchar)recievedData.at(4);
            timer.start();
            while (recievedData.length() < len && timer.elapsed() < 100) {
                recievedData += m_downloadPort.read(len - recievedData.length());
                QApplication::processEvents();
            }
        }else {
            return false;
        }

        if ((uchar)recievedData.at(recievedData.length() - 1) != PACKAGE_END) {return false;} //只做简单判断

        if (300000 < baud && baud < 350000) {
            timer1Value = 65536.2 - 24000000.0 / 4 / baud;
        }else {
            timer1Value = 65536.5 - 24000000.0 / 4 / baud;
        }

        if (m_mcuInfo.oscillator_Hz >= 35500000) { //35M以上
            ocValue = 0x30;
            ocValue2 = (m_mcuInfo.oscillator_Hz - 32120000) / (32120000 * 0.0024);
        }else if (m_mcuInfo.oscillator_Hz < 35500000 && m_mcuInfo.oscillator_Hz > 34500000) { //35M
            ocValue = 0x20;
            ocValue2 = 0xEC;
        }else if (m_mcuInfo.oscillator_Hz < 33700000 && m_mcuInfo.oscillator_Hz > 32700000) { //33.1776M
            ocValue = 0x20;
            ocValue2 = 0xCE;
        }else if (m_mcuInfo.oscillator_Hz < 30500000 && m_mcuInfo.oscillator_Hz > 29500000) { //30M
            ocValue = 0x20;
            ocValue2 = 0x99;
        }else if (m_mcuInfo.oscillator_Hz < 27500000 && m_mcuInfo.oscillator_Hz > 26500000) { //27M
            ocValue = 0x20;
            ocValue2 = 0x67;
        }else if (m_mcuInfo.oscillator_Hz < 24500000 && m_mcuInfo.oscillator_Hz > 23500000) { //24M
            ocValue = 0x20;
            ocValue2 = 0x34;
        }else if (m_mcuInfo.oscillator_Hz < 22700000 && m_mcuInfo.oscillator_Hz > 21700000) { //22.1184M
            ocValue = 0x20;
            ocValue2 = 0x15;
        }else if (m_mcuInfo.oscillator_Hz < 20500000 && m_mcuInfo.oscillator_Hz > 19500000) { //20M
            ocValue = 0x30;
            ocValue2 = 0x63;
        }else if (m_mcuInfo.oscillator_Hz < 18800000 && m_mcuInfo.oscillator_Hz > 18000000) { //18.432M
            ocValue = 0x30;
            ocValue2 = 0x46;
        }else if (m_mcuInfo.oscillator_Hz < 12300000 && m_mcuInfo.oscillator_Hz > 11700000) { //12M
            ocValue = 0x10;
            ocValue2 = 0xBB;
        }else if (m_mcuInfo.oscillator_Hz < 6300000 && m_mcuInfo.oscillator_Hz > 5700000) { //6M
            ocValue = 0x00;
            ocValue2 = 0x60;
        }else if (m_mcuInfo.oscillator_Hz < 5900000 && m_mcuInfo.oscillator_Hz > 5000000) { //5.5296M
            ocValue = 0x00;
            ocValue2 = 0x3F;
        }else { //m_mcuInfo.oscillator_Hz < 11400000 && m_mcuInfo.oscillator_Hz > 10800000 //默认值 11.0592M
            ocValue = 0x10;
            ocValue2 = 0x93;
        }

        //第二个握手包
        uchar ch = ocValue2 - 2;
        data.clear();
        data.append(0x0C);
        for(int i = 12; i > 0; --i) {
            data.append(ch++);
            data.append(ocValue);
        }
        this->send(0x00, data);
        m_downloadPort.waitForBytesWritten(200);
        m_downloadPort.write("\xFE");
        timeout = 10;
        recievedData.clear();
        timer.start();
        while (recievedData.length() < 5 && timer.elapsed() < 1000) {
            m_downloadPort.write("\xFE");
            while (timer.elapsed() < timeout) {
                QApplication::processEvents();
            }
            recievedData += m_downloadPort.read(5 - recievedData.length());
            timeout += 10;
        }

        if(!recievedData.length()) {return false;}
        timer.start();
        while (timer.elapsed() < 50) {
            QApplication::processEvents();
        }

        if (recievedData.length() >= 5) {
            len = 2 + ((uchar)recievedData.at(3) << 8) + (uchar)recievedData.at(4);
            timer.start();
            while (recievedData.length() < len && timer.elapsed() < 200) {
                recievedData += m_downloadPort.read(len - recievedData.length());
                QApplication::processEvents();
            }
        }else {
            return false;
        }

        if ((uchar)recievedData.at(recievedData.length() - 1) != PACKAGE_END) {return false;} //只做简单判断

        data.clear();
        data.append(2, 0x00);
        data.append(timer1Value >> 8);
        data.append(timer1Value);
        data.append(ocValue);
        data.append(ocValue2);
        data.append(0x98);

        //设置新波特率
        this->send(PACKAGE_CMD_SET_BAUDRATE_8_15, data);
        m_downloadPort.waitForBytesWritten(200);
        this->recieve();
        m_downloadPort.setBaudRate(baud);
    }else if (NS_McuData::Protocol_8 == m_mcuInfo.protocol) {
        if (300000 < baud && baud < 350000) {
            timer1Value = 65536.2 - 24000000.0 / 4 / baud;
        }else {
            timer1Value = 65536.5 - 24000000.0 / 4 / baud;
        }

        if (m_mcuInfo.oscillator_Hz < 27500000 && m_mcuInfo.oscillator_Hz > 26500000) { //27M
            ocValue = 0xB0;
        }else if (m_mcuInfo.oscillator_Hz < 24500000 && m_mcuInfo.oscillator_Hz > 23500000) { //24M
            ocValue = 0x7B;
        }else if (m_mcuInfo.oscillator_Hz < 22700000 && m_mcuInfo.oscillator_Hz > 21700000) { //22.1184M
            ocValue = 0x5A;
        }else if (m_mcuInfo.oscillator_Hz < 20500000 && m_mcuInfo.oscillator_Hz > 19500000) { //20M
            ocValue = 0x35;
        }else if (m_mcuInfo.oscillator_Hz < 18800000 && m_mcuInfo.oscillator_Hz > 18000000) { //18.432M
            ocValue = 0x1A;
        }else if (m_mcuInfo.oscillator_Hz < 12300000 && m_mcuInfo.oscillator_Hz > 11700000) { //12M
            ocValue = 0x7B;
        }else if (m_mcuInfo.oscillator_Hz < 11400000 && m_mcuInfo.oscillator_Hz > 10800000) { //11.0592M
            ocValue = 0x5A;
        }else if (m_mcuInfo.oscillator_Hz < 6300000 && m_mcuInfo.oscillator_Hz > 5700000) { //6M
            ocValue = 0x12;
        }else if (m_mcuInfo.oscillator_Hz < 5900000 && m_mcuInfo.oscillator_Hz > 5000000) { //5.5296M
            ocValue = 0x5A;
        }else {
            ocValue = 0x6B;
        }

        data.append(2, 0x00);
        data.append(timer1Value >> 8);
        data.append(timer1Value);
        data.append(0x01);
        data.append(ocValue);
        data.append(0x81);

        //设置新波特率
        this->send(PACKAGE_CMD_SET_BAUDRATE_8_15, data);
        m_downloadPort.waitForBytesWritten(200);
        this->recieve();
        m_downloadPort.setBaudRate(baud);

    }else if (NS_McuData::Protocol_15 == m_mcuInfo.protocol) {
        if (300000 < baud && baud < 350000) {
            timer1Value = 65536.2 - 22118400.0 / 4 / baud;
        }else {
            timer1Value = 65536.5 - 22118400.0 / 4 / baud;
        }

        if (m_mcuInfo.oscillator_Hz < 27500000 && m_mcuInfo.oscillator_Hz > 26500000) { //27M
            ocValue = 0x40;
            ocValue2 = 0xDC;
        }else if (m_mcuInfo.oscillator_Hz < 24500000 && m_mcuInfo.oscillator_Hz > 23500000) { //24M
            ocValue = 0x40;
            ocValue2 = 0x9F;
        }else if (m_mcuInfo.oscillator_Hz < 22700000 && m_mcuInfo.oscillator_Hz > 21700000) { //22.1184M
            ocValue = 0x40;
            ocValue2 = 0x79;
        }else if (m_mcuInfo.oscillator_Hz < 20500000 && m_mcuInfo.oscillator_Hz > 19500000) { //20M
            ocValue = 0x40;
            ocValue2 = 0x4F;
        }else if (m_mcuInfo.oscillator_Hz < 18800000 && m_mcuInfo.oscillator_Hz > 18000000) { //18.432M
            ocValue = 0x40;
            ocValue2 = 0x31;
        }else if (m_mcuInfo.oscillator_Hz < 12300000 && m_mcuInfo.oscillator_Hz > 11700000) { //12M
            ocValue = 0x80;
            ocValue2 = 0xA2;
        }else if (m_mcuInfo.oscillator_Hz < 11400000 && m_mcuInfo.oscillator_Hz > 10800000) { //11.0592M
            ocValue = 0x80;
            ocValue2 = 0x7D;
        }else if (m_mcuInfo.oscillator_Hz < 6300000 && m_mcuInfo.oscillator_Hz > 5700000) { //6M
            ocValue = 0xC0;
            ocValue2 = 0x9F;
        }else if (m_mcuInfo.oscillator_Hz < 5900000 && m_mcuInfo.oscillator_Hz > 5000000) { //5.5296M
            ocValue = 0xC0;
            ocValue2 = 0x7B;
        }else { //自行增加内容，防止编译警告，默认采用：11.0592MHz
            ocValue = 0x80;
            ocValue2 = 0x7D;
        }

        data = "\x6D\x40";
        data.append(timer1Value >> 8);
        data.append(timer1Value);
        data.append(ocValue);
        data.append(ocValue2);
        data.append(0x81);

        //设置新波特率
        this->send(PACKAGE_CMD_SET_BAUDRATE_8_15, data);
        m_downloadPort.waitForBytesWritten(200);
        this->recieve();
        m_downloadPort.setBaudRate(baud);
    }else {
        int i = NS_McuData::baudrateList.indexOf("115200"), j;
        float t;
        for (; i >= 0; --i) {
            baud = NS_McuData::baudrateList.at(i).toUInt();
            t = m_mcuInfo.oscillator_Hz / 32.0 / baud;
            if (NS_McuData::Protocol_89 != m_mcuInfo.protocol) {
                t *= 2;
            }

            if (qAbs(t - int(t + 0.5)) / t > 0.03) {continue;}

            if (NS_McuData::Protocol_89 == m_mcuInfo.protocol) {
                timer1Value = 0x10000 - int(t + 0.5);
            }else {
                if (t > 0xFF) {continue;}
                timer1Value = 0xC000 + 0x100 - int(t + 0.5);
            }

            data.clear();
            data.append(timer1Value >> 8);
            data.append(timer1Value);
            data.append(0xFF - (timer1Value >> 8));
            data.append(qMin((256 - (timer1Value & 0xFF)) << 1, uint(0xFE)));
            data.append(baud0 / 60);

            QList <quint32> freqList;
            if (NS_McuData::Protocol_89 == m_mcuInfo.protocol) {
                freqList << 40000000 << 20000000 << 10000000 << 5000000;
            }else {
                freqList << 30000000 << 24000000 << 20000000 << 12000000 << 6000000 << 3000000 << 2000000 << 1000000;
            }

            for (j = 0; j < freqList.size(); ++j) {
                if (m_mcuInfo.oscillator_Hz > freqList.at(j)) {break;}
            }

            data.append(0x80 + j);
            this->send(PACKAGE_CMD_TEST_BAUDRATE, data);
            m_downloadPort.waitForBytesWritten(200);

            timer.start();
            while (timer.elapsed() < 200) { //延时 200ms 等待发送完成
                QApplication::processEvents();
            }

            m_downloadPort.setBaudRate(baud);
            j = this->recieve().length();
            m_downloadPort.setBaudRate(baud0);
            if (j) {
                data.chop(1);
                this->send(PACKAGE_CMD_SET_BAUDRATE, data);
                m_downloadPort.waitForBytesWritten(200);

                timer.start();
                while (timer.elapsed() < 200) {
                    QApplication::processEvents();
                }

                m_downloadPort.setBaudRate(baud);
                return this->recieve().length();
            }else {
                return false;
            }
        }
    }

    return true;
}

bool MainWindow::erase()
{
    QByteArray data;
    bool succeed = true;
    ui->outputText->appendPlainText(tr("开始擦除芯片..."));

    if (NS_McuData::Protocol_89 == m_mcuInfo.protocol) {
        this->send(PACKAGE_CMD_ERASE, "\x01\x33\x33\x33\x33\x33\x33");
        m_downloadPort.waitForBytesWritten(100);
        data = this->recieve(10000);
        if (PACKEGE_CMD_CHECK != (uchar)data.at(5)) {succeed = false;}
    }else if (NS_McuData::Protocol_8H4K == m_mcuInfo.protocol || NS_McuData::Protocol_8 == m_mcuInfo.protocol || NS_McuData::Protocol_15 == m_mcuInfo.protocol) {
        data.append(2, 0x00);
        data += "\x5A\xA5";

        this->send(PACKAGE_CMD_ERASE_8_15, data);
        m_downloadPort.waitForBytesWritten(100);
        succeed = this->recieve(10000).length();

        this->send(PACKAGE_CMD_GET_SN_8_15, data);
        m_downloadPort.waitForBytesWritten(100);

        data = this->recieve(10000);

        if (data.length() >= 14) {
            QString sn;
            for (int i = 6; i < 6 + 7; ++i) {
                sn += QString("%1").arg((uchar)data.at(i), 2, 16, QLatin1Char('0'));
            }

            ui->outputText->insertPlainText(tr("完成\n芯片出厂序列号：%1\n").arg(sn).toUpper());
        }else {
            succeed = false;
        }
    }else {
        data.append(2, 0x00);
        data.append(m_mcuInfo.romSize_KB << 2);
        data.append(2, 0x00);
        data.append(m_mcuInfo.romSize_KB << 2);
        data.append(12, 0x00);
        uchar ch = 0x80;
        while (ch > 0x0D) {
            data.append(ch--);
        }
        this->send(PACKAGE_CMD_ERASE, data);
        m_downloadPort.waitForBytesWritten(100);
        data = this->recieve(10000);
        if (!data.length()) {
            succeed = false;
        }else {
            ui->outputText->insertPlainText(tr("完成\n"));
        }
    }

    if (!succeed) {
        ui->outputText->insertPlainText(tr("擦除失败"));
    }

    return succeed;
}

bool MainWindow::flash(const QByteArray &code)
{
    QByteArray addr;
    uint len = code.length(), prevProgress = 0, progress;

    for (uint i = 0; i < len; i += 128) {
        addr.clear();
        if (NS_McuData::Protocol_8H4K == m_mcuInfo.protocol) {
            addr.append(i >> 8);
            addr.append(i);
            addr.append("\x5A\xA5");
            this->send(PACKAGE_CMD_PROGRAM_8H4K, addr + code.mid(i, 128));
        }else if (NS_McuData::Protocol_8 == m_mcuInfo.protocol || NS_McuData::Protocol_15 == m_mcuInfo.protocol) {
            addr.append(i >> 8);
            addr.append(i);
            addr.append("\x5A\xA5");
            this->send(PACKAGE_CMD_PROGRAM_8_15, addr + code.mid(i, 128));
        }else {
            addr.append(2, 0x00);
            addr.append(i >> 8);
            addr.append(i);
            addr.append(1, 0x00);
            addr.append(0x80);
            this->send(PACKAGE_CMD_PROGRAM, addr + code.mid(i, 128));
        }

        m_downloadPort.waitForBytesWritten(1200);

        if (!this->recieve().length()) {
            return false;
        }

        //显示进度
        progress = (i + 128) * 25 / len;
        ui->outputText->insertPlainText(QString(progress - prevProgress, '#'));
        prevProgress = progress;
        QApplication::processEvents();
    }

    return true;
}

bool MainWindow::setOptions(const QByteArray &info)
{
    QByteArray data;
    if (ui->check_reaseEEPROM->isChecked()) {
        if (NS_McuData::Protocol_89 == m_mcuInfo.protocol) {
            data += info.at(2) & 0xF7;
            data.append(3, 0xFF);
        }else if (NS_McuData::Protocol_12C5A == m_mcuInfo.protocol) {
            data = info.mid(6, 3);
            data.append(5, 0xFF);
            data += info.at(10) & 0xFD;
            data.append(6, 0xFF);
            data.append(m_mcuInfo.oscillator_Hz >> 24);
            data.append(m_mcuInfo.oscillator_Hz >> 16);
            data.append(m_mcuInfo.oscillator_Hz >> 8);
            data.append(m_mcuInfo.oscillator_Hz);
        }else if (NS_McuData::Protocol_12C52 == m_mcuInfo.protocol || NS_McuData::Protocol_12Cx052 == m_mcuInfo.protocol) {
            data = info.mid(6, 5);
            data[2] = data.at(2) & 0xFD; //info.at(8)
            data.append(m_mcuInfo.oscillator_Hz >> 24);
            data.append(m_mcuInfo.oscillator_Hz >> 16);
            data.append(m_mcuInfo.oscillator_Hz >> 8);
            data.append(m_mcuInfo.oscillator_Hz);
            data += info.mid(12, 4);
            data.append(4, 0xFF);
            data += data.at(2); //info.at(8)
            data.append(7, 0xFF);
            data += data.mid(5, 4); //m_mcuInfo.oscillator_Hz
            data.append(3, 0xFF);
        }else {
            return false;
        }
    }

    if (data.length()) {
        this->send(PACKAGE_CMD_SET_OPTIONS, data);
        m_downloadPort.waitForBytesWritten(300);
        return this->recieve().length();
    }

    return true;
}

void MainWindow::terminate()
{
    uchar cmd = PACKAGE_CMD_TERMINATE;
    if (NS_McuData::Protocol_8 == m_mcuInfo.protocol || NS_McuData::Protocol_15 == m_mcuInfo.protocol) {
        cmd = PACKAGE_CMD_TERMINATE_8_15;
    }

    this->send(cmd, QByteArray());
    m_downloadPort.waitForBytesWritten(100);
    QElapsedTimer timer;
    timer.start();
    while (timer.elapsed() < 200) {
        QApplication::processEvents();
    }
}

bool MainWindow::unkownPacket_1()
{
    QByteArray data;
    if (NS_McuData::Protocol_12C5A == m_mcuInfo.protocol || NS_McuData::Protocol_12C52 == m_mcuInfo.protocol) {
        m_downloadPort.readAll();
        data.append(2, 0x00);
        data += "\x36\x01" + m_mcuInfo.info.mid(3, 2);
        this->send(0x50, data);
        data = this->recieve();

        if (data.length() < 6 || 0x8F != (uchar)data.at(5)) {
            return false;
        }
    }

    return true;
}

bool MainWindow::unkownPacket_2()
{
    QByteArray data, sendData;
    sendData.append(2, 0x00);
    sendData += "\x36\x01" + m_mcuInfo.info.mid(3, 2);
    if (NS_McuData::Protocol_12C5A != m_mcuInfo.protocol
        && NS_McuData::Protocol_12C52 != m_mcuInfo.protocol
        && NS_McuData::Protocol_8 != m_mcuInfo.protocol
        && NS_McuData::Protocol_15 != m_mcuInfo.protocol
        && NS_McuData::Protocol_8H4K != m_mcuInfo.protocol)
    {
        for (int i = 0; i < 5; ++i) {
            this->send(0x80, sendData);
            m_downloadPort.waitForBytesWritten(100);
            data = this->recieve();
            if (data.length() < 6 || 0x80 != (uchar)data.at(5) ) {
                return false;
            }
        }
    }

    return true;
}

bool MainWindow::unkownPacket_3()
{
    QByteArray data, sendData;
    sendData.append(2, 0x00);
    sendData += "\x36\x01" + m_mcuInfo.info.mid(3, 2);
    if (NS_McuData::Protocol_12C5A == m_mcuInfo.protocol || NS_McuData::Protocol_12C52 == m_mcuInfo.protocol) {
        this->send(0x69, sendData);
        m_downloadPort.waitForBytesWritten(100);
        data = this->recieve();
        if (data.length() < 6 || 0x8D != (uchar)data.at(5)) {
            return false;
        }
    }

    return true;
}

void MainWindow::program()
{
    if (tr("停止") == ui->btn_download->text()) {
        m_stopped = true;
        ui->btn_download->setText(tr("下载"));
        return;
    }

    QFileInfo info(m_file);
    if (!info.exists() || !info.isFile()) {
        QMessageBox::information(this, tr("提示"), tr("请选择文件"), tr("确定"));
        return;
    }

    QByteArray data;
    bool succeed = true;

    if (info.suffix().toLower() == "hex" || info.suffix().toLower() == "ihx") {
        data = this->hex2bin(m_file, &succeed);
    }else {
        QFile file(m_file);
        if (file.open(QIODevice::ReadOnly)) {
            data = file.readAll();
            file.close();
        }else {
            succeed = false;
        }
    }
    if (!succeed) {
        QMessageBox::critical(this, tr("错误"), tr("无法读取文件“%1”").arg(m_file), tr("确定"));
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
    QTime time;
    NS_McuData::Protocol selectedProtocol;
    int index = ui->combo_models->currentIndex();

    if (index <= END_INDEX_89) {
        selectedProtocol = NS_McuData::Protocol_89;
    }else if (index <= END_INDEX_12C5A) {
        selectedProtocol = NS_McuData::Protocol_12C5A;
    }else if (index <= END_INDEX_12C52) {
        selectedProtocol = NS_McuData::Protocol_12C52;
    }else if (index <= END_INDEX_12Cx052) {
        selectedProtocol = NS_McuData::Protocol_12Cx052;
    }else if (index <= END_INDEX_15) {
        selectedProtocol = NS_McuData::Protocol_15;
    }else if (index <= END_INDEX_8) {
        selectedProtocol = NS_McuData::Protocol_8;
    }else if (index <= END_INDEX_8H4K) {
        selectedProtocol = NS_McuData::Protocol_8H4K;
    }else {
        selectedProtocol = NS_McuData::Protocol_None;
    }

    if (!m_downloadPort.open(QIODevice::ReadWrite)) {
        QMessageBox::critical(this, tr("错误"), errorMessage(m_downloadPort.error()), tr("确定"));
        return;
    }

    ui->outputText->clear();
    ui->tabWidget->setCurrentIndex(0);
    ui->btn_download->setText(tr("停止"));
    m_stopped = false;
    ui->outputText->appendPlainText(tr("检测目标..."));
    if (this->detect()) {
        ui->outputText->insertPlainText(tr("完成"));
    }else {
        ui->outputText->insertPlainText(tr("失败"));
        goto final;
    }

    if (NS_McuData::Protocol_None == m_mcuInfo.protocol) {
        if (m_mcuInfo.info.length() > 4) {
            ui->outputText->appendPlainText(tr("未知型号：%1 %2").arg((uchar)m_mcuInfo.info.at(3), 2, 16, QLatin1Char('0')).arg((uchar)m_mcuInfo.info.at(4), 2, 16, QLatin1Char('0')).toUpper());
        }

        goto final;
    }

    this->printInfo();

    if (selectedProtocol != NS_McuData::Protocol_None && selectedProtocol != m_mcuInfo.protocol) {
        ui->outputText->appendPlainText(tr("\n下载失败：型号不匹配"));
        goto final;
    }

    if (this->unkownPacket_1()) {
        ui->outputText->appendPlainText(tr("切换至最高波特率: "));
    }else {
        ui->outputText->appendPlainText(tr("下载失败：握手未完成"));
        goto final;
    }

    if (this->handshake()) {
        ui->outputText->insertPlainText(QString("%1 bps").arg(m_downloadPort.baudRate()));
    }else {
        ui->outputText->appendPlainText(tr("下载失败：握手未完成"));
        goto final;;
    }

    if (!this->unkownPacket_2()) {
        ui->outputText->appendPlainText(tr("下载失败：握手未完成"));
        goto final;;
    }

    time = QTime::currentTime();
    if (!this->erase()) {
        ui->outputText->appendPlainText(tr("下载失败"));
        goto final;
    }

    ui->outputText->appendPlainText(tr("代码长度：%1 bytes\n正在下载用户代码...").arg(data.length()));
    if (data.length() & 0x7F) {
        data.append(0x80 - (data.length() & 0x7F), 0xFF);
    }

    if (this->flash(data)) {
        ui->outputText->insertPlainText(tr("完成"));
    }else {
        ui->outputText->insertPlainText(tr("下载失败"));
        goto final;
    }

    if (this->unkownPacket_3()) {
        ui->outputText->appendPlainText(tr("设置选项..."));
        if (this->setOptions(m_mcuInfo.info)) {
            ui->outputText->insertPlainText(tr("设置完成"));
        }else {
            ui->outputText->insertPlainText(tr("设置失败"));
        }
    }

    this->terminate();
    ui->outputText->appendPlainText(tr("耗时：%1s").arg(time.msecsTo(QTime::currentTime()) / 1000.0, 0, 'f', 3));

final:
    m_downloadPort.close();
    ui->btn_download->setText(tr("下载"));
}
