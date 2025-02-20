#include <protocol.h>


PDU *mkPDU(uint uiMsgLen) // make PDU
{
    uint uiPDULen = sizeof(PDU) + uiMsgLen;
    PDU *pdu = (PDU*)malloc(uiPDULen); // 申请空间
    if(pdu == nullptr){
        exit(EXIT_FAILURE); // 申请空间失败则退出程序
    }
    memset(pdu, 0, uiPDULen); // 数据初始化为0
    pdu->uiPDULen = uiPDULen; // 数据参数初始化
    pdu->uiMsgLen = uiMsgLen;
    return pdu;
}
