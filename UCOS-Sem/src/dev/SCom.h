#ifndef __SCOM_H__
#define __SCOM_H__

typedef enum
{
    DEV_PARAM_TIMER = 0x10, //0x10
    DEV_PARAM_COMM_MODE,    //0x11
    DEV_PARAM_WIRELESS,     //0x12
    DEV_PARAM_SRV_NET,      //0x13
    DEV_PARAM_DEV_NET,      //0x14
    DEV_PARAM_WORK_MODE,    //0x15
    DEV_PARAM_SERIAL,       //0x16
    DEV_PARAM_MOB_NUM,      //0x17
    DEV_PARAM_HEART_PRID,   //0x18
    DEV_PARAM_REP_SIGN,     //0x19
    DEV_PARAM_READ_TIME,    //0x1a
    DEV_PARAM_METER_CNT,    //0x1b
    DEV_PARAM_METER_PRM,    //0x1c
    DEV_PARAM_INVALID_0,    //0x1d
    DEV_PARAM_FLOW_RATE,    //0x1e
    DEV_PARAM_EVENT_SIGN,   //0x1f
    DEV_PARAM_DEV_EVENT,    //0x20
    DEV_PARAM_VERSION,      //0x21
    DEV_PARAM_USER_INFO,    //0x22
    DEV_PARAM_DEV_ADDR,     //0x23
    DEV_PARAM_SLEEP_TIME,   //0x24
    DEV_PARAM_NET_INFO,     //0x25
    DEV_PARAM_IP_MODE,      //0x26
    DEV_PARAM_MAC_ADDR,     //0x27
    DEV_PARAM_ENV_INFO = 0x30,   
} PARAM_TYPE_CODE;

#define CMD_TIMEOUT   10  // 10√Î

#define TCP_DataBuf   Recv645Buf
#define TCP_BufLen    Recv_645Save_num
#define TCP_sav_num  Recv_645Save_num

#define TCP_SENDLEN   SENDBUFLEN
#define TCP_SendBuf   Send645Buf

extern  BYTE NeedSaveParam ;
extern  BYTE NeedReboot ;
extern BYTE NeedRebootChk ;

BYTE RecValidTcpFrame(void);
BYTE HandleTcpFrame(BYTE Frm );
void LoginDevice(void);
void LogoutDevice(void);
void InvalidRequest(void);
void GetDevWirelessParam(void);
void ResetDevice(void);
void GetDevTimer(void);
void GetDevCommMode(void);
void GetSrvNetParam(void);
void GetDevNetParam(void);
void GetDevWorkMode(void);
void GetDevSerialParam(void);
void GetDevMobNumber(void);
void GetDevHeartTime(void);
void GetDevRepSign(void);
void GetDevReadTime(void);
void GetDevMeterCount(void);
void GetDevMeterParam(void);
void GetDevFlowRate(void);
void GetDevEventSign(void);
void GetDevEvent(void);
void GetDevVersion(void);
void ChangePassword(void);
void RepDevStatus(void);
BYTE RepHeartPack(void);
void RepDevData(void);
void SetDevTime(void);
void SetDevCommMode(void);
void SetDevWirelessParam(void);
void SetSrvNetParam(void);
void SetDevNetParam(void);
void SetDevWorkMode(void);
void SetDevSerialParam(void);
void SetDevMobNumber(void);
void SetDevHeartTime(void);
void SetDevRepSign(void);
void SetDevReadTime(void);
void SetDevMeterCount(void);
void SetDevMeterParam(void);
void ClearDevFlowRate(void);

void RetDevData(void);
void RetHeartPack(void);
void RetDevStatus(void);
void GetDevUserInfo(void);
void GetDevSleepTime(void);
void GetDevNetInfo(void);
void GetDevIpMode(void);
void GetDevMacAddr(void);

void SetDevUserInfo(void);
void SetDevAddress(void);
void SetDevSleepTime(void);
void SetDevNetInfo(void);
void SetDevIpMode(void);

void SleepDevice(void);
void ServerHandle(void);
//BYTE CommHandle(void);

void Clear_Sms_buf(void);

void sendModTestStrToRs485(void);
void sendTestStrToRs485(void);
void HandleDevFrame(void);
void SysSleep(void);


#endif
