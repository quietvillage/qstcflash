//文件 mcudata.h
//关于芯片的配置信息：芯片型号、烧录协议、支持的波特率...

#ifndef MCU_DATA_H
#define MCU_DATA_H

#include <QStringList>

#define PACKAGE_HEAD0                   0x46 //包头两个字节
#define PACKAGE_HEAD1                   0xB9
#define PACKAGE_FLAG_UP_STREAM          0x68 //表示来自单片机的数据包
#define PACKAGE_FLAG_DOWN_STREAM        0x6A
#define PACKAGE_CMD_DETECT              0x7F
#define PACKAGE_CMD_TEST_BAUDRATE       0x8F //测试波特率 STC89、90、10、11、12系列
#define PACKAGE_CMD_SET_BAUDRATE        0x8E //设置波特率 STC89、90、10、11、12系列
#define PACKAGE_CMD_SET_BAUDRATE_8_15   0x01 //设置波特率 STC8、15系列
#define PACKAGE_CMD_ERASE               0x84
#define PACKAGE_CMD_ERASE_8_15          0x05
#define PACKAGE_CMD_GET_SN_8_15         0x03
#define PACKEGE_CMD_CHECK               0x80 //校验擦除是否成功
#define PACKAGE_CMD_TERMINATE_8_15      0xFF
#define PACKAGE_CMD_TERMINATE           0x82 //结束编程
#define PACKAGE_CMD_SET_OPTIONS_8_15    0x04
#define PACKAGE_CMD_SET_OPTIONS         0x8D
#define PACKAGE_CMD_PROGRAM_8_15        0x22
#define PACKAGE_CMD_PROGRAM_8H4K        0x32
#define PACKAGE_CMD_PROGRAM             0x00
#define PACKAGE_END                     0x16

//协议结束（芯片型号下拉选择框）索引值，增加型号时，相同协议的型号须紧挨在一起，不能交叉排列
#define END_INDEX_89        1
#define END_INDEX_12C5A     4
#define END_INDEX_12C52     6
#define END_INDEX_12Cx052   7
#define END_INDEX_15        8
#define END_INDEX_8         9
#define END_INDEX_8H4K      10

namespace NS_McuData {

enum Protocol {
    Protocol_None       =   0,
    Protocol_89         =   1,
    Protocol_12C5A      =   2,
    Protocol_12C52      =   3,
    Protocol_12Cx052    =   4,
    Protocol_15         =   5,
    Protocol_8          =   6,
    Protocol_8H4K       =   7
};

struct McuInfo {
    enum Protocol protocol;
    quint32 romSize_KB;
    quint32 oscillator_Hz;
    quint32 wakeupOscillator_Hz;
    quint32 innerVref_mV;   //内部参考电压
    float lowVoltage_V;   //低电压检测值
    int checkMode; //校验和字节数
    QByteArray info;  //探测时返回的芯片相关信息
    QString testDate;
    QString version;

    McuInfo() {
        protocol = Protocol_None;
        romSize_KB = 0;
        oscillator_Hz = 0;
        wakeupOscillator_Hz = 0;
        innerVref_mV = 0;
        lowVoltage_V = 0.0;
        checkMode = 0;
    }
};

struct McuModel {
    int romRatio;
    uchar base; //rom 倍数基准值
    uchar end; //rom 倍数值(含)
    QString name; //芯片具体型号
};

extern const QStringList protocolList;
extern const QStringList baudrateList; //支持的波特率
extern const QMap<int, struct McuModel> mcuModelMap;//芯片型号列表

enum Protocol protocol(uint type);
int mcuModelMapKey(uint type, uint value);

}
#endif // MCU_DATA_H
