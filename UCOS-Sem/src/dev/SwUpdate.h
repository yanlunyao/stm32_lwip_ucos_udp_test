/*************************************************************
成都昊普环保技术有限公司   版权所有

文件名:  SwUpdate.h
作  者:  潘国义
描  述:  软件升级模块
修订记录:   

**************************************************************/

#ifndef __SWUPDATA_H__
#define __SWUPDATA_H__

#define FLASH_BANK_SIZE 1024

void SwUpdateRequest(void);
void SwUpdateData(void);
void SwUpdateFinish(void);
void SoftWareUpdate(void);
void ClearSwDownloadInfo(void);
#endif
