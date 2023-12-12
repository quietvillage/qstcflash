#include "mcudata.h"
#include <QObject>
#include <QMap>


const QStringList NS_McuData::protocolList = {
    "STC89C5xxRC", //89
    "STC89C5xxRD+", //89
    "STC10Fxx", //12C5A
    "STC11Fxx", //12C5A
    "STC12C5Axx", //12C5A
    "STC12C52xx", //12C52
    "STC12C56xx", //12C52
    "STC12Cxx052", //12Cx052
    "STC15xx", //15
    "STC8xx", //8
    "STC8H4Kxx", //8H4K
    QObject::tr("自动判断") //None
};

const QStringList NS_McuData::baudrateList = {
    "1200",
    "2400",
    "4800",
    "9600",
    "19200",
    "38400",
    "57600",
    "115200",
    "230400",
    "460800"
};

const QMap<int, struct NS_McuData::McuModel> NS_McuData::mcuModelMap = {
    //key, {romRatio, base, end, name}
    {0xE000, {1, 0x00, 0x1F, "12C54%1"}},
    {0xE060, {1, 0x60, 0x7F, "12C54%1AD"}},
    {0xE080, {1, 0x80, 0x9F, "12LE54%1"}},
    {0xE0E0, {1, 0xE0, 0xFF, "12LE54%1AD"}},

    {0xE100, {1, 0x00, 0x1F, "12C52%1"}},
    {0xE120, {1, 0x20, 0x3F, "12C52%1PWM"}},
    {0xE160, {1, 0x60, 0x7F, "12C52%1AD"}},
    {0xE180, {1, 0x80, 0x9F, "12LE52%1"}},
    {0xE1A0, {1, 0xA0, 0xBF, "12LE52%1PWM"}},
    {0xE1E0, {1, 0xE0, 0xFF, "12LE52%1AD"}},

    {0xE200, {1, 0x00, 0x1F, "11F%1"}},
    {0xE220, {1, 0x20, 0x3F, "11F%1E"}},
    {0xE270, {1, 0x70, 0x75, "11F%1"}},
    {0xE275, {1, 0x70, 0x76, "IAP11F%1"}},
    {0xE276, {1, 0x70, 0x7F, "11F%1"}},
    {0xE280, {1, 0x80, 0x9F, "11L%1"}},
    {0xE2A0, {1, 0xA0, 0xBF, "11L%1E"}},
    {0xE2F0, {1, 0xF0, 0xF5, "11L%1"}},
    {0xE2F5, {1, 0xF0, 0xF6, "IAP11L%1"}},
    {0xE2F6, {1, 0xF0, 0xFF, "11L%1"}},

    {0xE600, {1, 0x00, 0x1F, "12C56%1"}},
    {0xE660, {1, 0x60, 0x7F, "12C56%1AD"}},
    {0xE680, {1, 0x80, 0x9F, "12LE56%1"}},
    {0xE6E0, {1, 0xE0, 0xFF, "12LE56%1AD"}},

    {0xD120, {2, 0x20, 0x3E, "12C5A%1CCP"}},
    {0xD13E, {2, 0x20, 0x3F, "IAP12C5A%1CCP"}},
    {0xD140, {2, 0x40, 0x5E, "12C5A%1AD"}},
    {0xD15E, {2, 0x40, 0x5F, "IAP12C5A%1AD"}},
    {0xD160, {2, 0x60, 0x7E, "12C5A%1S2"}},
    {0xD17E, {2, 0x60, 0x7F, "IAP12C5A%1S2"}},
    {0xD1A0, {2, 0xA0, 0xBF, "12LE5A%1CCP"}},
    {0xD1C0, {2, 0xC0, 0xDF, "12LE5A%1AD"}},
    {0xD1E0, {2, 0xE0, 0xFF, "12LE5A%1S2"}},

    {0xD200, {1, 0x00, 0x0F, "10F%1"}},
    {0xD260, {1, 0x60, 0x6F, "10F%1XE"}},
    {0xD770, {1, 0x70, 0x7D, "10L%1X"}},
    {0xD27D, {1, 0x70, 0x7E, "IAP10L%1X"}},
    {0xD27E, {1, 0x70, 0x7F, "10L%1X"}},
    {0xD2A0, {1, 0xA0, 0xAF, "10L%1"}},
    {0xD2E0, {1, 0xE0, 0xEF, "10L%1XE"}},
    {0xD2F0, {1, 0xF0, 0xFD, "10L%1X"}},
    {0xD2FD, {1, 0xF0, 0xFE, "IAP10L%1X"}},
    {0xD2FE, {1, 0xF0, 0xFF, "10L%1X"}},

    {0xD300, {2, 0x00, 0x1F, "11F%1"}},
    {0xD340, {2, 0x40, 0x5E, "11F%1X"}},
    {0xD35E, {2, 0x40, 0x5F, "IAP11F%1X"}},
    {0xD360, {2, 0x60, 0x7F, "11L%1XE"}},
    {0xD3A0, {2, 0xA0, 0xBF, "11L%1"}},
    {0xD3C0, {2, 0xC0, 0xDE, "11L%1X"}},
    {0xD3DE, {2, 0xC0, 0xDF, "IAP11L%1X"}},
    {0xD3E0, {2, 0xE0, 0xFF, "11L%1XE"}},

    {0xF000, {4, 0x00, 0x02, "89C5%1RC"}},
    {0xF002, {13, 0x02, 0x03, "89C5%1RC"}},
    {0xF003, {4, 0x00, 0x10, "89C5%1RC"}},
    {0xF020, {4, 0x20, 0x30, "90C5%1RC"}},
    {0xF100, {4, 0x00, 0x10, "89C5%1RD+"}},
    {0xF120, {4, 0x20, 0x30, "90C5%1RD+"}},

    {0xF200, {1, 0x00, 0x0F, "12C%1052"}},
    {0xF210, {1, 0x10, 0x1F, "12C%1052AD"}},
    {0xF220, {1, 0x20, 0x2F, "12LE%1052"}},
    {0xF230, {1, 0x30, 0x3F, "12LE%1052AD"}},
    {0xF2A0, {1, 0xA0, 0xA5, "15W1%1"}},

    {0xF400, {8, 0x00, 0x06, "15F2K%1S2"}},
    {0xF407, {60, 0x07, 0x08, "15F2K%1S2"}},
    {0xF408, {61, 0x08, 0x09, "IAP15F2K%1S2"}},
    {0xF409, {4, 0x09, 0x0C, "15F4%1AD"}},
    {0xF410, {8, 0x10, 0x17, "15F1K%1AS"}},
    {0xF417, {60, 0x17, 0x18, "15F1K%1AS"}},
    {0xF418, {61, 0x18, 0x19, "IAP15F1K%1AS"}},
    {0xF420, {8, 0x20, 0x27, "15F1K%1S"}},
    {0xF427, {60, 0x27, 0x28, "15F1K%1S"}},
    {0xF440, {8, 0x40, 0x47, "15F1K%1S2"}},
    {0xF447, {60, 0x47, 0x48, "15F1K%1S2"}},
    {0xF448, {61, 0x48, 0x49, "IAP15F1K%1S2"}},
    {0xF44C, {13, 0x4C, 0x4D, "IAP15F4%1AD"}},
    {0xF450, {8, 0x50, 0x57, "15F1K%1AS"}},
    {0xF457, {60, 0x57, 0x58, "15F1K%1AS"}},
    {0xF458, {61, 0x58, 0x59, "IAP15F1K%1AS"}},
    {0xF460, {8, 0x60, 0x67, "15F1K%1KS"}},
    {0xF467, {60, 0x67, 0x68, "15F1K%1KS"}},
    {0xF468, {61, 0x68, 0x69, "IAP15F1K%1KS"}},
    {0xF480, {8, 0x80, 0x87, "15L2K%1S2"}},
    {0xF487, {60, 0x87, 0x88, "15L2K%1S2"}},
    {0xF488, {61, 0x88, 0x89, "IAP15L2K%1S2"}},
    {0xF489, {5, 0x89, 0x8C, "15L4%1AD"}},
    {0xF490, {8, 0x90, 0x97, "15L2K%1AS"}},
    {0xF497, {60, 0x97, 0x98, "15L2K%1AS"}},
    {0xF498, {61, 0x98, 0x99, "IAP15L2K%1AS"}},
    {0xF4A0, {8, 0xA0, 0xA7, "15L2K%1S"}},
    {0xF4A7, {60, 0xA7, 0xA8, "15L2K%1S"}},
    {0xF4A8, {61, 0xA8, 0xA9, "IAP15L2K%1S"}},
    {0xF4C0, {8, 0xC0, 0xC7, "15L1K%1S2"}},
    {0xF4C7, {60, 0xC7, 0xC8, "15L1K%1S2"}},
    {0xF4C8, {61, 0xC8, 0xC9, "IAP15L1K%1S2"}},
    {0xF4CC, {13, 0xCC, 0xCD, "IAP15L4%1AD"}},
    {0xF4D0, {8, 0xD0, 0xD7, "15L1K%1AS"}},
    {0xF4D7, {60, 0xD7, 0xD8, "15L1K%1AS"}},
    {0xF4D8, {61, 0xD8, 0xD9, "IAP15L1K%1AS"}},
    {0xF4E0, {8, 0xE0, 0xE7, "15L1K%1S"}},
    {0xF4E7, {60, 0xE7, 0xE8, "15L1K%1S"}},
    {0xF4E8, {61, 0xE8, 0xE9, "IAP15L1K%1S"}},

    {0xF500, {1, 0x00, 0x04, "15W1%1SW"}},
    {0xF507, {1, 0x07, 0x0B, "15W1%1S"}},
    {0xF510, {1, 0x10, 0x14, "15W2%1S"}},
    {0xF514, {8, 0x14, 0x17, "15W1K%1S"}},
    {0xF518, {4, 0x18, 0x1A, "15W4%1S"}},
    {0xF51A, {4, 0x1A, 0x1C, "15W4%1S"}},
    {0xF51C, {4, 0x1C, 0x1F, "15W4%1AS"}},
    {0xF51F, {10, 0x1F, 0x20, "15W4%1AS"}},
    {0xF520, {12, 0x20, 0x21, "15W4K%1S4"}},
    {0xF521, {8, 0x21, 0x28, "15W4K%1S4"}},
    {0xF529, {1, 0x29, 0x2B, "15W4%1A4"}},
    {0xF52C, {8, 0x2C, 0x2E, "15W1K%1PWM"}},
    {0xF52E, {20, 0x2E, 0x2F, "15W1K%1S"}},
    {0xF52F, {32, 0x2F, 0x30, "15W2K%1S2"}},
    {0xF530, {48, 0x30, 0x31, "15W2K%1S2"}},
    {0xF531, {32, 0x31, 0x32, "15W2K%1S2"}},
    {0xF533, {20, 0x33, 0x34, "15W1K%1S2"}},
    {0xF534, {32, 0x34, 0x35, "15W1K%1S2"}},
    {0xF535, {48, 0x35, 0x36, "15W1K%1S2"}},
    {0xF544, {5, 0x44, 0x45, "IAP15W%1SW"}},
    {0xF554, {5, 0x54, 0x55, "IAP15W2%1S"}},
    {0xF557, {29, 0x57, 0x58, "IAP15W1K%1S"}},
    {0xF55C, {13, 0x5C, 0x5D, "IAP15W4%1S"}},
    {0xF568, {58, 0x68, 0x69, "IAP15W4K%1S4"}},
    {0xF569, {61, 0x69, 0x6A, "IAP15W4K%1S4"}},
    {0xF56C, {58, 0x6C, 0x6D, "IAP15W4K%1S4-Student"}},
    {0xF57E, {8, 0x7E, 0x85, "15U4K%1S4"}},

    {0xF600, {8, 0x00, 0x08, "15H4K%1S4"}},

    {0xF620, {8, 0x20, 0x28, "8A8K%1S4A12"}},
    {0xF628, {60, 0x28, 0x29, "8A8K%1S4A12"}},
    {0xF630, {8, 0x30, 0x38, "8F2K%1S4"}},
    {0xF638, {60, 0x38, 0x39, "8F2K%1S4"}},
    {0xF640, {8, 0x40, 0x48, "8F2K%1S2"}},
    {0xF648, {60, 0x48, 0x49, "8F2K%1S2"}},
    {0xF650, {8, 0x50, 0x58, "8A4K%1S2A12"}},
    {0xF658, {60, 0x58, 0x59, "8A4K%1S2A12"}},
    {0xF660, {2, 0x60, 0x66, "8F1K%1S2"}},
    {0xF666, {17, 0x66, 0x67, "8F1K%1S2"}},
    {0xF670, {2, 0x70, 0x76, "8F1K%1"}},
    {0xF676, {17, 0x76, 0x77, "8F1K%1"}},

    {0xF700, {2, 0x00, 0x06, "8C1K%1"}},
    {0xF730, {2, 0x30, 0x36, "8H1K%1"}},
    {0xF736, {17, 0x36, 0x37, "8H1K%1"}},
    {0xF740, {8, 0x40, 0x42, "8H3K%1S4"}},
    {0xF742, {60, 0x42, 0x43, "8H3K%1S4"}},
    {0xF743, {64, 0x43, 0x44, "8H3K%1S4"}},
    {0xF748, {16, 0x48, 0x4A, "8H3K%1S2"}},
    {0xF74A, {60, 0x4A, 0x4B, "8H3K%1S2"}},
    {0xF74B, {64, 0x4B, 0x4C, "8H3K%1S2"}},
    {0xF750, {2, 0x50, 0x56, "8G1K%1-20/16pin"}},
    {0xF756, {17, 0x56, 0x57, "8G1K%1-20/16pin"}},
    {0xF760, {16, 0x60, 0x62, "8G2K%1S4"}},
    {0xF762, {60, 0x62, 0x63, "8G2K%1S4"}},
    {0xF763, {64, 0x63, 0x64, "8G2K%1S4"}},
    {0xF768, {16, 0x68, 0x6A, "8G2K%1S2"}},
    {0xF76A, {60, 0x6A, 0x6B, "8G2K%1S2"}},
    {0xF76B, {64, 0x6B, 0x6C, "8G2K%1S2"}},
    {0xF770, {2, 0x70, 0x76, "8G1K%1T"}},
    {0xF776, {17, 0x76, 0x77, "8G1K%1T"}},
    {0xF780, {16, 0x80, 0x82, "8H8K%1U"}},
    {0xF782, {60, 0x82, 0x83, "8H8K%1U"}},
    {0xF783, {64, 0x83, 0x84, "8H8K%1U"}},
    {0xF790, {2, 0x90, 0x96, "8G1K%1A-8PIN"}},
    {0xF796, {17, 0x96, 0x97, "8G1K%1A-8PIN"}},
    {0xF7A0, {2, 0xA0, 0xA6, "8G1K%1-8PIN"}},
    {0xF7A6, {17, 0xA6, 0xA7, "8G1K%1-8PIN"}},

    {0xF830, {16, 0x30, 0x34, "8H4K%1TL"}}
};

enum NS_McuData::Protocol NS_McuData::protocol(uint type)
{
    enum Protocol ret = Protocol_None;
    switch (type) {
    case 0xF0:
    case 0xF1:
        ret = Protocol_89;
        break;
    case 0xD1:
    case 0xD2:
    case 0xD3:
    case 0xE2:
        ret = Protocol_12C5A;
        break;
    case 0xE0:
    case 0xE1:
    case 0xE6:
        ret = Protocol_12C52;
        break;
    case 0xF2:
        ret = Protocol_12Cx052;
        break;
    case 0xF4:
    case 0xF5:
        ret = Protocol_15;
        break;
    case 0xF6:
    case 0xF7:
        ret = Protocol_8;
        break;
    case 0xF8:
        ret = Protocol_8H4K;
        break;
    }

    return ret;
}

int NS_McuData::mcuModelMapKey(uint type, uint value)
{
    int ret = -1;
    switch (type) {
    case 0xF8:
        if (value >= 0x30 && value <= 0x34) {
            ret = 0xF830;
        }
        break;
    case 0xF7:
        if (value <= 0x06U) {
            ret = 0xF700;
        }else if (value <= 0x36) {
            ret = 0xF730;
        }else if (value <= 0x37) {
            ret = 0xF736;
        }else if (value <= 0x42) {
            ret = 0xF740;
        }else if (value <= 0x43) {
            ret = 0xF742;
        }else if (value <= 0x44) {
            ret = 0xF743;
        }else if (value <= 0x4A) {
            ret = 0xF748;
        }else if (value <= 0x4B) {
            ret = 0xF74A;
        }else if (value <= 0x4C) {
            ret = 0xF74B;
        }else if (value <= 0x56) {
            ret = 0xF750;
        }else if (value <= 0x58) {
            ret = 0xF756;
        }else if (value <= 0x62) {
            ret = 0xF760;
        }else if (value <= 0x63) {
            ret = 0xF762;
        }else if (value <= 0x64) {
            ret = 0xF763;
        }else if (value <= 0x6A) {
            ret = 0xF768;
        }else if (value <= 0x6B) {
            ret = 0xF76A;
        }else if (value <= 0x6C) {
            ret = 0xF76B;
        }else if (value <= 0x76) {
            ret = 0xF770;
        }else if (value <= 0x77) {
            ret = 0xF776;
        }else if (value <= 0x82) {
            ret = 0xF780;
        }else if (value <= 0x83) {
            ret = 0xF782;
        }else if (value <= 0x84) {
            ret = 0xF783;
        }else if (value <= 0x96) {
            ret = 0xF790;
        }else if (value <= 0x97) {
            ret = 0xF796;
        }else if (value <= 0xA6) {
            ret = 0xF7A0;
        }else if (value <= 0xA7) {
            ret = 0xF7A6;
        }
        break;
    case 0xF6:
        if (value <= 0x08) {
            ret = 0xF600;
        }else if (value <= 0x28) {
            ret = 0xF620;
        }else if (value <= 0x29) {
            ret = 0xF628;
        }else if (value <= 0x38) {
            ret = 0xF630;
        }else if (value <= 0x39) {
            ret = 0xF638;
        }else if (value <= 0x48) {
            ret = 0xF640;
        }else if (value <= 0x49) {
            ret = 0xF648;
        }else if (value <= 0x58) {
            ret = 0xF650;
        }else if (value <= 0x59) {
            ret = 0xF658;
        }else if (value <= 0x66) {
            ret = 0xF660;
        }else if (value <= 0x67) {
            ret = 0xF666;
        }else if (value <= 0x76) {
            ret = 0xF670;
        }else if (value <= 0x77) {
            ret = 0xF676;
        }
        break;
    case 0xF5:
        if (value <= 0x04) {
            ret = 0xF500;
        }else if (value <= 0x0B) {
            ret = 0xF507;
        }else if (value <= 0x14) {
            ret = 0xF510;
        }else if (value <= 0x17) {
            ret = 0xF514;
        }else if (value <= 0x1A) {
            ret = 0xF518;
        }else if (value <= 0x1C) {
            ret = 0xF51A;
        }else if (value <= 0x1F) {
            ret = 0xF51C;
        }else if (value <= 0x20) {
            ret = 0xF51F;
        }else if (value <= 0x21) {
            ret = 0xF520;
        }else if (value <= 0x28) {
            ret = 0xF522;
        }else if (value <= 0x2B) {
            ret = 0xF529;
        }else if (value <= 0x2E) {
            ret = 0xF52C;
        }else if (value <= 0x2F) {
            ret = 0xF52E;
        }else if (value <= 0x30) {
            ret = 0xF52F;
        }else if (value <= 0x31) {
            ret = 0xF530;
        }else if (value <= 0x32) {
            ret = 0xF531;
        }else if (value <= 0x34) {
            ret = 0xF533;
        }else if (value <= 0x35) {
            ret = 0xF534;
        }else if (value <= 0x36) {
            ret = 0xF535;
        }else if (value <= 0x45) {
            ret = 0xF544;
        }else if (value <= 0x55) {
            ret = 0xF554;
        }else if (value <= 0x58) {
            ret = 0xF557;
        }else if (value <= 0x5D) {
            ret = 0xF55C;
        }else if (value <= 0x69) {
            ret = 0xF568;
        }else if (value <= 0x6A) {
            ret = 0xF569;
        }else if (value <= 0x6D) {
            ret = 0xF56C;
        }else if (value <= 0x85) {
            ret = 0xF57E;
        }
        break;
    case 0xF4:
        if (value <= 0x06) {
            ret = 0xF400;
        }else if (value <= 0x08) {
            ret = 0xF407;
        }else if (value <= 0x09) {
            ret = 0xF408;
        }else if (value <= 0x0C) {
            ret = 0xF409;
        }else if (value <= 0x17) {
            ret = 0xF410;
        }else if (value <= 0x18) {
            ret = 0xF417;
        }else if (value <= 0x19) {
            ret = 0xF418;
        }else if (value <= 0x27) {
            ret = 0xF420;
        }else if (value <= 0x28) {
            ret = 0xF427;
        }else if (value <= 0x47) {
            ret = 0xF440;
        }else if (value <= 0x48) {
            ret = 0xF447;
        }else if (value <= 0x49) {
            ret = 0xF448;
        }else if (value <= 0x4D) {
            ret = 0xF44C;
        }else if (value <= 0x57) {
            ret = 0xF450;
        }else if (value <= 0x58) {
            ret = 0xF457;
        }else if (value <= 0x59) {
            ret = 0xF458;
        }else if (value <= 0x67) {
            ret = 0xF460;
        }else if (value <= 0x68) {
            ret = 0xF467;
        }else if (value <= 0x69) {
            ret = 0xF468;
        }else if (value <= 0x87) {
            ret = 0xF480;
        }else if (value <= 0x88) {
            ret = 0xF487;
        }else if (value <= 0x89) {
            ret = 0xF488;
        }else if (value <= 0x8C) {
            ret = 0xF489;
        }else if (value <= 0x97) {
            ret = 0xF490;
        }else if (value <= 0x98) {
            ret = 0xF497;
        }else if (value <= 0x99) {
            ret = 0xF498;
        }else if (value <= 0xA7) {
            ret = 0xF4A0;
        }else if (value <= 0xA8) {
            ret = 0xF4A7;
        }else if (value <= 0xA9) {
            ret = 0xF4A8;
        }else if (value <= 0xC7) {
            ret = 0xF4C0;
        }else if (value <= 0xC8) {
            ret = 0xF4C7;
        }else if (value <= 0xC9) {
            ret = 0xF4C8;
        }else if (value <= 0xCD) {
            ret = 0xF4CC;
        }else if (value <= 0xD7) {
            ret = 0xF4D0;
        }else if (value <= 0xD8) {
            ret = 0xF4D7;
        }else if (value <= 0xD9) {
            ret = 0xF4D8;
        }else if (value <= 0xE7) {
            ret = 0xF4E0;
        }else if (value <= 0xE8) {
            ret = 0xF4E7;
        }else if (value <= 0xE9) {
            ret = 0xF4E8;
        }
        break;
    case 0xF2:
        if (value <= 0x0F) {
            ret = 0xF200;
        }else if (value <= 0x1F) {
            ret = 0xF210;
        }else if (value <= 0x2F) {
            ret = 0xF220;
        }else if (value <= 0x3F) {
            ret = 0xF230;
        }else if (value <= 0xA5) {
            ret = 0xF2A0;
        }
        break;
    case 0xF1:
        if (value <= 0x10) {
            ret = 0xF100;
        }else if (value <= 0x30) {
            ret = 0xF120;
        }
        break;
    case 0xF0:
        if (value <= 0x02) {
            ret = 0xF100;
        }else if (value <= 0x03) {
            ret = 0xF002;
        }else if (value <= 0x10) {
            ret = 0xF003;
        }else if (value <= 0x30) {
            ret = 0xF020;
        }
        break;
    case 0xE6:
        if (value <= 0x1F) {
            ret = 0xE600;
        }else if (value <= 0x7F) {
            ret = 0xE660;
        }else if (value <= 0x9F) {
            ret = 0xE680;
        }else/* if (value <= 0xFF) */{
            ret = 0xE6E0;
        }
        break;
    case 0xE2:
        if (value <= 0x1F) {
            ret = 0xE200;
        }else if (value <= 0x3F) {
            ret = 0xE220;
        }else if (value <= 0x75) {
            ret = 0xE270;
        }else if (value <= 0x76) {
            ret = 0xE275;
        }else if (value <= 0x7F) {
            ret = 0xE276;
        }else if (value <= 0x9F) {
            ret = 0xE280;
        }else if (value <= 0xBF) {
            ret = 0xE2A0;
        }else if (value <= 0xF5) {
            ret = 0xE2F0;
        }else if (value <= 0xF6) {
            ret = 0xE2F5;
        }else/* if (value <= 0xFF) */{
            ret = 0xE2F6;
        }
        break;
    case 0xE1:
        if (value <= 0x1F) {
            ret = 0xE100;
        }else if (value <= 0x3F) {
            ret = 0xE120;
        }else if (value <= 0x7F) {
            ret = 0xE160;
        }else if (value <= 0x9F) {
            ret = 0xE180;
        }else if (value <= 0xBF) {
            ret = 0xE1A0;
        }else/* if (value <= 0xFF) */{
            ret = 0xE1E0;
        }
        break;
    case 0xE0:
        if (value <= 0x1F) {
            ret = 0xE000;
        }else if (value <= 0x7F) {
            ret = 0xE060;
        }else if (value <= 0x9F) {
            ret = 0xE080;
        }else/* if (value <= 0xFF) */{
            ret = 0xE0E0;
        }
        break;
    case 0xD3:
        if (value <= 0x1F) {
            ret = 0xD300;
        }else if (value <= 0x5E) {
            ret = 0xD340;
        }else if (value <= 0x5F) {
            ret = 0xD35E;
        }else if (value <= 0x7F) {
            ret = 0xD360;
        }else if (value <= 0xBF) {
            ret = 0xD3A0;
        }else if (value <= 0xDE) {
            ret = 0xD3C0;
        }else if (value <= 0xDF) {
            ret = 0xD3DE;
        }else/* if (value <= 0xFF) */{
            ret = 0xD3E0;
        }
        break;
    case 0xD2:
        if (value <= 0x0F) {
            ret = 0xD200;
        }else if (value <= 0x6F) {
            ret = 0xD260;
        }else if (value <= 0x7D) {
            ret = 0xD270;
        }else if (value <= 0x7E) {
            ret = 0xD27D;
        }else if (value <= 0x7F) {
            ret = 0xD27E;
        }else if (value <= 0xAF) {
            ret = 0xD2A0;
        }else if (value <= 0xEF) {
            ret = 0xD2E0;
        }else if (value <= 0xFD) {
            ret = 0xD2F0;
        }else if (value <= 0xFE) {
            ret = 0xD2FD;
        }else /*if (value <= 0xFF)*/ {
            ret = 0xD2FE;
        }
        break;
    case 0xD1:
        if (value <= 0x3E) {
            ret = 0xD120;
        }else if (value <= 0x3F) {
            ret = 0xD13E;
        }else if (value <= 0x5E) {
            ret = 0xD140;
        }else if (value <= 0x5F) {
            ret = 0xD15E;
        }else if (value <= 0x7E) {
            ret = 0xD160;
        }else if (value <= 0x7F) {
            ret = 0xD17E;
        }else if (value <= 0xBF) {
            ret = 0xD1A0;
        }else if (value <= 0xDF) {
            ret = 0xD1C0;
        }else if (value <= 0xFE) {
            ret = 0xD1E0;
        }
        break;
    default:
        break;
    }

    return ret;
}
