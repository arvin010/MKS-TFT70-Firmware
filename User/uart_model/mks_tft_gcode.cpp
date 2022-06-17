/********************   (C) COPYRIGHT 2014 www.makerbase.com.cn   ********************
 * �ļ���  ��mks_tft_gcode.c
 * ����    ��1.��u�̶�ȡԴ�ļ���ÿ�ζ�ȡ1k�ֽڣ�����д��udiskBuffer.buffer[0]��udiskBuffer.buffer[1]��
 						2. ��udiskBuffer.buffer[n]�ǿ�ʱ����ȡ����Чgcodeָ�����ǰ/��׺��,Push��gcodeTxFIFO���С�
 * ����    ��skyblue
**********************************************************************************/


#include <stdio.h>
#include <string.h>
#include "ff.h"
#include "mks_tft_fifo.h"
#include "mks_tft_gcode.h"
//#include "main.h"
#include "mks_tft_com.h"
//**#include "printer.h"
//**#include "draw_ui.h"
#include "mks_cfg.h"
//#include "others.h"
#include "GUI.h"
#include "draw_dialog.h"
#include "draw_ui.h"
#include "at24cxx.h"
//#include "repetier.h"
//#include "HAL.h"
#include "sdio.h"
#include "fatfs.h"
#include "usb_host.h"
#include "tim.h"
#include "sd_usr.h"
#include "draw_print_file.h"
#include "string.h"


extern uint8_t IsChooseAutoShutdown;
extern uint8_t close_fail_flg;
extern uint16_t close_fail_cnt;
/***************************add******************/
extern CFG_ITMES gCfgItems;
extern PRINT_TIME  print_time;
//extern void Btn_putdown_close_machine();
extern uint32_t rePrintOffset;
extern char SD_Path[4];  /* SD logical drive path */

void mks_clearFile();

unsigned char path_bak[15]= {0};
#if defined(TFT70)
unsigned char *path_reprint = (unsigned char *)"/mks_pft35.sys";
#elif defined(TFT35)
unsigned char *path_reprint = (unsigned char *)"/mks_pft70.sys";
#endif
FIL fp_reprint_rw;
/***************************end******************/

struct position Gcode_current_position[30];

uint8_t Chk_close_machine_flg = 0;

UDISK_DATA_BUFFER udiskBuffer;

unsigned char note_flag=1;	//ע�ͱ�־ init : 1
unsigned long gcodeLineCnt=0;	//ָ���кű�� Nxxxxx


UDISK_FILE_STAUS udiskFileStaus;			//�ļ�״̬

TARGER_TEMP targetTemp;
TEMP_STATUS	tempStatus;
void getFanStatus(unsigned char *gcode,unsigned char *end)
{
	unsigned char tempBuf[30];
	unsigned char i;
	unsigned char *p;
	
		if(*gcode == 'M' && *(gcode+1) == '1' && *(gcode+2) == '0'&& (*(gcode+3) == '6' || *(gcode+3) == '7' ))	//M106 M107
		{
			p = gcode;
			i=0;
			while(p<end)
			{
				tempBuf[i++]=*p++;
			}
			tempBuf[i] = '\n';
			
			pushFIFO(&gcodeCmdRxFIFO,&tempBuf[0]);
		}
		
}

void getTargetTemp(unsigned char *gcode,unsigned char *end)
{
	int8_t *tmpStr_1 = 0;
	
	unsigned char tempBuf[80]="ok T:0 /210 B:0 /45 @:0 B@:0";
	unsigned char count;
	unsigned char *p;
	if(tempStatus == temp_ok )		return;
	
	p = &tempBuf[0];
	//��ȡ��λ mm or inch ,Ĭ��mm

	if(*gcode == 'G' && *(gcode+1) == '2' && *(gcode+2) == '0' )
		RePrintData.unit = 1;	//0 mm,1 inch
/*	
//20151019
	if(*gcode == 'M' && *(gcode+1) == '1' && (*(gcode+2) == '9' ||*(gcode+2) == '4' )&& *(gcode+3) == '0')	//M190 or M140
	{
		gcode += 4;
		count = 0;
		while(*gcode++ != 'S')
			if(count++ > 10) break;

		while(gcode < end)
		{
			if(*gcode == '.')break;
			*p++ = *gcode++;
			if(p >=&tempBuf[0]+10) break;
		}
		*p = '\0';
		targetTemp.bedTemp = atoi(&tempBuf[0]);
	}

	if(*gcode == 'M' && *(gcode+1) == '1' && *(gcode+2) == '0'&& (*(gcode+3) == '9' || *(gcode+3) == '4'))	//M109 or M104
	{
		gcode += 4;
		count = 0;
		while(*gcode++ != 'S')
			if(count++ > 10) break;

		while(gcode < end)
		{
			if(*gcode == '.') break;
			*p++ = *gcode++;
			if(p >=&tempBuf[0]+10) break;
		}
		*p = '\0';
		targetTemp.t0Temp = atoi(&tempBuf[0]);
	}
*/
	if(*gcode == 'M' && *(gcode+1) == '1' && (*(gcode+2) == '9' ||*(gcode+2) == '4' )&& *(gcode+3) == '0')	//M190 or M140
	{
		gcode += 4;
		count = 0;
		while(*gcode++ != 'S')
			if(count++ > 10) break;

		while(gcode < end)
		{
			if(*gcode == '.')break;
			*p++ = *gcode++;
			if(p >=&tempBuf[0]+10) break;
		}
		*p = '\0';
		targetTemp.bedTemp =atoi((const char *)&tempBuf[0]);
	}

	if(gCfgItems.sprayerNum == 1)
	{
		if(*gcode == 'M' && *(gcode+1) == '1' && *(gcode+2) == '0'&& (*(gcode+3) == '9' || *(gcode+3) == '4'))	//M109 or M104
		{
			gcode += 4;
			count = 0;
			while(*gcode++ != 'S')
				if(count++ > 10) break;

			while(gcode < end)
			{
				if(*gcode == '.') break;
				*p++ = *gcode++;
				if(p >=&tempBuf[0]+10) break;
			}
			*p = '\0';
			targetTemp.t0Temp =atoi((const char *)&tempBuf[0]);
		}
	}
	else
	{
		if(*gcode == 'M' && *(gcode+1) == '1' && *(gcode+2) == '0'&& (*(gcode+3) == '9' || *(gcode+3) == '4'))	//M109 or M104
		{
			if((int8_t *)strstr((const char *)gcode, (const char *)"T0"))
			{
					tmpStr_1 = (int8_t *)strstr((const char *)gcode, (const char *)"S");	
					if(tmpStr_1)
					{
						gcode = (unsigned char *)tmpStr_1+1;
						while(gcode < end)
						{
							if(*gcode == '.') break;
							*p++ = *gcode++;
							if(p >=&tempBuf[0]+10) break;
						}
						*p = '\0';
						targetTemp.t0Temp = atoi((const char *)&tempBuf[0]);						
					}
			}
			else if((int8_t *)strstr((const char *)gcode, (const char *)"T1"))
			{
					tmpStr_1 = (int8_t *)strstr((const char *)gcode, (const char *)"S");	
					if(tmpStr_1)
					{
						gcode = (unsigned char *)tmpStr_1+1;
						while(gcode < end)
						{
							if(*gcode == '.') break;
							*p++ = *gcode++;
							if(p >=&tempBuf[0]+10) break;
						}
						*p = '\0';
						targetTemp.t1Temp = atoi((const char *)&tempBuf[0]);						
					}			
			}
			else
			{
					tmpStr_1 = (int8_t *)strstr((const char *)gcode, (const char *)"S");	
					if(tmpStr_1)
					{
						gcode = (unsigned char *)tmpStr_1+1;
						while(gcode < end)
						{
							if(*gcode == '.') break;
							*p++ = *gcode++;
							if(p >=&tempBuf[0]+10) break;
						}
						*p = '\0';
						if(RePrintData.spayerchoose == 1)
						{
							targetTemp.t1Temp = atoi((const char *)&tempBuf[0]);
						}
						else
						{
							targetTemp.t0Temp = atoi((const char *)&tempBuf[0]);						
						}		
					}
			}		

		}

	}
/*
	if((targetTemp.bedTemp > 0 && targetTemp.t0Temp >0) ||( gcodeLineCnt> 50))
	{
		//tempBuf[40]="ok T:0 /210 B:0 /45 @:0 B@:0";
		p = &tempBuf[0];	
		*p++ = 'o';*p++ = 'k';*p++ = ' ';*p++ = 'T';*p++ = ':';*p++ = '0';*p++ = ' ';*p++ = '/';
		*p++ = targetTemp.t0Temp/100+48;
		*p++ = (targetTemp.t0Temp/10)%10 + 48;
		*p++ = targetTemp.t0Temp%10 + 48;
		
		*p++ = ' ';	*p++ = 'B';*p++ = ':';*p++ = '0';*p++ = ' ';*p++ = '/';
		*p++ = targetTemp.bedTemp/10+48;
		*p++ = targetTemp.bedTemp%10 + 48;
		*p++ = ' ';*p++ = '@';*p++ = ':';*p++ = '0';*p++ = ' ';*p++ = 'B';*p++ = '@';*p++ = ':';*p++ = '0';*p++ = '\n';
		
		pushFIFO(&gcodeCmdRxFIFO,&tempBuf[0]);
		tempStatus = temp_ok;
	}
*/
	if((targetTemp.bedTemp > 0)||(targetTemp.t0Temp >0)||(targetTemp.t1Temp >0))
	{
		if(gCfgItems.sprayerNum == 1)
		{
			//tempBuf[40]="ok T:0 /210 B:0 /45 @:0 B@:0";
			p = &tempBuf[0];	
			*p++ = 'o';*p++ = 'k';*p++ = ' ';*p++ = 'T';*p++ = ':';
			// *p++ = '0';
			*p++ = ((uint32_t)(gCfgItems.curSprayerTemp[0]))/100+48;
			*p++ = (((uint32_t)(gCfgItems.curSprayerTemp[0]))/10)%10 + 48;
			*p++ = ((uint32_t)(gCfgItems.curSprayerTemp[0]))%10 + 48;
			*p++ = ' ';*p++ = '/';
			*p++ = targetTemp.t0Temp/100+48;
			*p++ = (targetTemp.t0Temp/10)%10 + 48;
			*p++ = targetTemp.t0Temp%10 + 48;
			
			*p++ = ' ';	*p++ = 'B';*p++ = ':';
			// *p++ = '0';
			*p++ = ((uint32_t)(gCfgItems.curBedTemp))/100+48;
			*p++ = (((uint32_t)(gCfgItems.curBedTemp))/10)%10+48;
			*p++ = ((uint32_t)(gCfgItems.curBedTemp))%10 + 48;
			*p++ = ' ';*p++ = '/';
			*p++ = targetTemp.bedTemp/100+48;
			*p++ = (targetTemp.bedTemp/10)%10+48;
			*p++ = targetTemp.bedTemp%10 + 48;
			*p++ = ' ';*p++ = '@';*p++ = ':';*p++ = '0';*p++ = ' ';*p++ = 'B';*p++ = '@';*p++ = ':';*p++ = '0';*p++ = '\n';
			
			pushFIFO(&gcodeCmdRxFIFO,&tempBuf[0]);
		}
		else
		{
			//ok T:0 /210 B:0 /45 T0:0/210 T1:0 /210 @:0 B@:0
			p = &tempBuf[0];	
			*p++ = 'o';*p++ = 'k';*p++ = ' ';*p++ = 'T';*p++ = ':';
			// *p++ = '0';
			*p++ = ((uint32_t)(gCfgItems.curSprayerTemp[0]))/100+48;
			*p++ = (((uint32_t)(gCfgItems.curSprayerTemp[0]))/10)%10 + 48;
			*p++ = ((uint32_t)(gCfgItems.curSprayerTemp[0]))%10 + 48;
			*p++ = ' ';*p++ = '/';
			*p++ = targetTemp.t0Temp/100+48;
			*p++ = (targetTemp.t0Temp/10)%10 + 48;
			*p++ = targetTemp.t0Temp%10 + 48;
			//B
			*p++ = ' ';	*p++ = 'B';*p++ = ':';
			// *p++ = '0';
			*p++ = ((uint32_t)(gCfgItems.curBedTemp))/100+48;
			*p++ = (((uint32_t)(gCfgItems.curBedTemp))/10)%10+48;
			*p++ = ((uint32_t)(gCfgItems.curBedTemp))%10 + 48;
			*p++ = ' ';*p++ = '/';
			*p++ = targetTemp.bedTemp/100+48;
			*p++ = (targetTemp.bedTemp/10)%10+48;
			*p++ = targetTemp.bedTemp%10 + 48;
			//T0
			*p++ = ' ';*p++ = 'T';*p++ = '0';*p++ = ':';
			// *p++ = '0';
			*p++ = ((uint32_t)(gCfgItems.curSprayerTemp[0]))/100+48;
			*p++ = (((uint32_t)(gCfgItems.curSprayerTemp[0]))/10)%10 + 48;
			*p++ = ((uint32_t)(gCfgItems.curSprayerTemp[0]))%10 + 48;			
			*p++ = ' ';*p++ = '/';
			*p++ = targetTemp.t0Temp/100+48;
			*p++ = (targetTemp.t0Temp/10)%10 + 48;
			*p++ = targetTemp.t0Temp%10 + 48;
			//T1
			*p++ = ' ';*p++ = 'T';*p++ = '1';*p++ = ':';
			// *p++ = '0';
			*p++ = ((uint32_t)(gCfgItems.curSprayerTemp[1]))/100+48;
			*p++ = (((uint32_t)(gCfgItems.curSprayerTemp[1]))/10)%10 + 48;
			*p++ = ((uint32_t)(gCfgItems.curSprayerTemp[1]))%10 + 48;			
			*p++ = ' ';*p++ = '/';
			*p++ = targetTemp.t1Temp/100+48;
			*p++ = (targetTemp.t1Temp/10)%10 + 48;
			*p++ = targetTemp.t1Temp%10 + 48;

			
			*p++ = ' ';*p++ = '@';*p++ = ':';*p++ = '0';*p++ = ' ';*p++ = 'B';*p++ = '@';*p++ = ':';*p++ = '0';*p++ = '\n';

			pushFIFO(&gcodeCmdRxFIFO,&tempBuf[0]);			
		}
		
	}

	if(gcodeLineCnt> 50)
	{
		tempStatus = temp_ok;
	}

}

volatile RECOVER_SD rec_sd;
void udiskBufferInit(void)
{
	memset(udiskBuffer.buffer[0],'\n',sizeof(udiskBuffer.buffer[0]));
	memset(udiskBuffer.buffer[1],'\n',sizeof(udiskBuffer.buffer[1]));

	memset(udiskBuffer.mid_buffer[0],'\n',sizeof(udiskBuffer.mid_buffer[0]));
	memset(udiskBuffer.mid_buffer[1],'\n',sizeof(udiskBuffer.mid_buffer[1]));
	
	udiskBuffer.current = 0;
	udiskBuffer.p = udiskBuffer.buffer[udiskBuffer.current];
	udiskBuffer.state[udiskBuffer.current] = udisk_buf_full;
	udiskBuffer.state[(udiskBuffer.current+1)%2] = udisk_buf_empty;

	note_flag = 1;
	gcodeLineCnt = 0;
	RePrintData.record_line = 0;
	
	udiskFileStaus = udisk_file_ok;
	/*----------------*/
	targetTemp.bedTemp = 0;
	targetTemp.t0Temp = 0;
	targetTemp.t1Temp = 0;
	targetTemp.t2Temp = 0;
	tempStatus = temp_fail;
	/*----------------*/

	RePrintData.saveEnable = 0;
	
	initFIFO(&gcodeTxFIFO);
//	initFIFO(&gcodeRxFIFO);
	initFIFO(&gcodeCmdTxFIFO);
	initFIFO(&gcodeCmdRxFIFO);

	//chen 10.8
	rec_sd.just_delay_one = 0;
	rec_sd.total= 0;
}

//DWORD DDKK=0;

void udiskFileR(FIL *srcfp1)		//��ȡu���ļ���д��udiskBuffer
{	
		unsigned int readByteCnt=0;
		//unsigned int readByteCnt_Mid=0;
		volatile FRESULT res;
		DWORD   fptr_Int,fptr_remainder,srcfp_fptr;
		
		
		if((udiskBuffer.state[(udiskBuffer.current+1)%2] == udisk_buf_full) && (udiskFileStaus == udisk_file_ok))
			return;
	
		switch(udiskFileStaus)
		{
			case udisk_file_ok:
					#if 1
					/*sean test start*/
					//f_lseek(srcfp,500); 
					//while(1)
					//{
					/*sean test end*/
					if((srcfp)->fptr % 512 !=0)
					{
					#if 1
						memset(udiskBuffer.mid_buffer[0],'\n',sizeof(udiskBuffer.mid_buffer[0]));
					
						/*sean test start*/
						//memset(udiskBuffer.buffer[(udiskBuffer.current+1)%2],'\n',sizeof(udiskBuffer.buffer[(udiskBuffer.current+1)%2]));
						/*sean test end*/
						
						srcfp_fptr=(srcfp)->fptr ;   //�����ļ�λ��ƫ��
						fptr_Int=(srcfp)->fptr /512;   //ȡ��
						fptr_remainder=(srcfp)->fptr%512; //ȡ��
						f_lseek(srcfp,fptr_Int*512);  //��λ��512������λ��
						
						
						if(((srcfp)->fsize - fptr_Int*512) <= UDISKBUFLEN)   //�Ƿ�Ϊ���1K����?
						{
							res=f_read(srcfp,udiskBuffer.mid_buffer[0],((srcfp)->fsize - fptr_Int*512),&readByteCnt); //512��������ʣ���ֽ�
							strncpy((char *)udiskBuffer.buffer[(udiskBuffer.current+1)%2],(const char *)&udiskBuffer.mid_buffer[0][fptr_remainder-1],UDISKBUFLEN-fptr_remainder+1);//copy����
							if(res == FR_OK)
							{
								(srcfp)->fptr =(srcfp)->fsize;   // �ļ�ƫ�Ƶ����ļ���С
							}
						}
						else
						{
							res=f_read(srcfp,udiskBuffer.mid_buffer[0],UDISKBUFLEN,&readByteCnt);//�ȶ�1k
							strncpy((char *)udiskBuffer.buffer[(udiskBuffer.current+1)%2],(const char *)&udiskBuffer.mid_buffer[0][fptr_remainder-1],UDISKBUFLEN-fptr_remainder+1);//copy����
							if(res == FR_OK)
							{
								//rec_sd.total += readByteCnt;
								
							}

							memset(udiskBuffer.mid_buffer[0],'\n',sizeof(udiskBuffer.mid_buffer[0]));
							
							res=f_read(srcfp,udiskBuffer.mid_buffer[0],UDISKBUFLEN,&readByteCnt);//�ٶ�1k
							strncpy((char *)&udiskBuffer.buffer[(udiskBuffer.current+1)%2][UDISKBUFLEN-fptr_remainder+1],(const char *)udiskBuffer.mid_buffer[0],fptr_remainder-1);//copy����
							//�������ֽڱ�������,����������Ҫ�������ļ�ƫ�ƶ�λΪԭ��λ��+1K
							if(readByteCnt > fptr_remainder-1)f_lseek(srcfp,srcfp_fptr+UDISKBUFLEN);
							else if(res == FR_OK)(srcfp)->fptr =(srcfp)->fsize;   // �ļ�ƫ�Ƶ����ļ���С
						}
		
						/*for(int i=0;i<UDISKBUFLEN;i++)
						{
							if(i<(UDISKBUFLEN-fptr_remainder))
								udiskBuffer.buffer[(udiskBuffer.current+1)%2][i]=udiskBuffer.mid_buffer[0][i+fptr_remainder];
							else 
								udiskBuffer.buffer[(udiskBuffer.current+1)%2][i]=udiskBuffer.mid_buffer[1][i-(UDISKBUFLEN-fptr_remainder)];
						}

						if(readByteCnt_Mid < UDISKBUFLEN )
							f_lseek(srcfp,srcfp_fptr+readByteCnt_Mid);
						else
							f_lseek(srcfp,srcfp_fptr+UDISKBUFLEN);*/	
					#else
						if(((srcfp)->fptr%512)>256)
						{
							fptr_remainder=(srcfp)->fptr + (512-(srcfp)->fptr%512);
							f_lseek(srcfp,fptr_remainder);
						}
						else
						{
							fptr_remainder=(srcfp)->fptr -((srcfp)->fptr%512);
							f_lseek(srcfp,fptr_remainder);
						}
						f_read(srcfp,udiskBuffer.buffer[(udiskBuffer.current+1)%2],UDISKBUFLEN,&readByteCnt);

					#endif
						
					}
					else
					{
						res=f_read(srcfp,udiskBuffer.buffer[(udiskBuffer.current+1)%2],UDISKBUFLEN,&readByteCnt);
					}
					/*sean test start*/
					//}
					/*sean test end*/
					//sean 12.1
					if(res == FR_OK)
					{
						//rec_sd.total += readByteCnt;
						
						udiskBuffer.state[(udiskBuffer.current+1)%2] = udisk_buf_full;

						if((srcfp)->fptr >=(srcfp)->fsize )
						{
							udiskFileStaus = udisk_file_end;
							fileEndCnt = 30000;	
						}
					}
					else
					{	
						memset(udiskBuffer.buffer[(udiskBuffer.current+1)%2],'\n',sizeof(udiskBuffer.buffer[(udiskBuffer.current+1)%2]));				

						//if(srcfp->fsize > rec_sd.total)
						{
							//chen 9.29
							//���յ�ok���ٷ���gcode����
							usart2Data.prWaitStatus = pr_wait_idle;
							
							if(rec_sd.just_delay_one == 0)
							tftDelay(2000);  //��ֹ�ظ���ʱ
							
							rec_sd.just_delay_one = 1;
							
							Restart_data_init();
						} 
					}
					
					#endif

				break;
			case udisk_file_end:
					
					if((udiskBuffer.state[0] == udisk_buf_empty && udiskBuffer.state[1] == udisk_buf_empty && checkFIFO(&gcodeTxFIFO)== fifo_empty)) //��ӡ����
					{
						tftDelay(3);
						printerInit();
						tftDelay(3);

						//**I2C_EE_Init(100000);
						MX_I2C1_Init(100000);   //**

						HAL::AT24CXX_Read(BAK_REPRINT_INFO, (uint8_t *)&dataToEeprom,  4); 
						dataToEeprom &= 0x00ffffff;
						dataToEeprom |= (uint32_t)(printer_normal << 24 ) & 0xff000000;
						HAL::AT24CXX_Write(BAK_REPRINT_INFO,(uint8_t *)&dataToEeprom ,  4); 		// �����־(uint8_t) | ��λunit (uint8_t) | saveFlag(uint8_t)| null(uint8_t)
						
						printerStaus = pr_idle;		//��ӡ����
						usart2Data.printer = printer_idle;
						usart2Data.prWaitStatus = pr_wait_idle;
						usart2Data.timer = timer_stop;						//�����ʱ��

						#ifdef SAVE_FROM_SD
						//ɾ�����������ļ���
						if(gCfgItems.pwroff_save_mode == 0)
							mks_clearFile();

						/*
						if(gCfgItems.pwroff_save_mode == 0)
						{
							if(gCfgItems.fileSysType == FILE_SYS_SD)
							{
								strcpy((char *)path_bak, "1:");
								strcat((char *)path_bak,(const char *)path_reprint);						
								f_unlink((const char *)path_bak);
							}
							else
							{
								strcpy((char *)path_bak, "0:");
								strcat((char *)path_bak,(const char *)path_reprint);						
								f_unlink((const char *)path_bak);
							}
						}
						*/
						#endif

						if((gCfgItems.print_finish_close_machine_flg == 1)&&(IsChooseAutoShutdown == 1))
						{
							//Print_finish_close_machine();
							Btn_putdown_close_machine();
							IsChooseAutoShutdown = 0;
							clear_cur_ui();
							#if 0
							//GUI_SetFont(&FONT_TITLE);
							if(gCfgItems.language == LANG_COMPLEX_CHINESE)
							{
								GUI_SetFont(&GUI_FontHZ16);
							}
							else
							{
								GUI_SetFont(&FONT_TITLE);
							}

							if(gCfgItems.language == LANG_ENGLISH)
							{
								GUI_DispStringAt("Print end! Closing Machine...", 300, 200);
							}
							else 	if(gCfgItems.language == LANG_COMPLEX_CHINESE)
							{
								GUI_DispStringAt("��ӡ���!�����P�C...", 300, 200);
							}
							else
							{
								GUI_DispStringAt("��ӡ���! ���ڹػ�...", 300, 200);
							}
							#endif
							GUI_DispStringAt(common_menu.close_machine_tips, 300, 200);
							close_fail_flg = 1;
							close_fail_cnt = 0;
							while(close_fail_flg);
							clear_cur_ui();
							draw_dialog(DIALOG_TYPE_M80_FAIL);
						}
					}

					if(udiskBuffer.state[udiskBuffer.current] == udisk_buf_empty)
					{
							udiskBuffer.current = (udiskBuffer.current+1)%2;
							udiskBuffer.p = udiskBuffer.buffer[udiskBuffer.current];
					}

				break;
				default : break;
		}
}

volatile FRESULT res_ok;
extern FATFS fs;
extern int8_t curDirLever;
extern DIR_OFFSET dir_offset[10];
void Restart_data_init()
{
	volatile uint8_t record_cnt;
	volatile uint16_t last_tick;
	
	//memset(gCurDir, 0, sizeof(gCurDir));
		
	f_close(srcfp);
	curDirLever = 0;	
	dir_offset[curDirLever].curPage = 0;

	sd.Sd_file_offset = 0;
	sd.Sd_file_cnt = 0;

	memset(dir_offset, 0, sizeof(dir_offset));
		
      memset(sd.gCurDir, 0, sizeof(sd.gCurDir));
      #if _LFN_UNICODE
      sd.gCurDir[0] = '1';
      sd.gCurDir[1] = ':';
      #else
        if((gCfgItems.fileSysType == FILE_SYS_SD))
        {
		if(SD_DET_IP == SDCARDDETECTINVERTED)
		{
        		FATFS_LinkDriver_sd(&SD_Driver, SD_Path);
			f_mount(&fs, (TCHAR const*)SD_Path, 0);	
		}
		strcpy((char *)sd.gCurDir, "1:");      //skyblue-modify
        }
        else
        {
		FATFS_LinkDriver_usb(&USBH_Driver, USBH_Path);
	       f_mount(&fs, (TCHAR const*)USBH_Path, 0);
		strcpy((char *)sd.gCurDir, "0:");			
        }
		
	#endif
	    

	res_ok=f_open(srcfp,curFileName,FA_READ);
	if(res_ok == FR_OK)
	{	
		rec_sd.just_delay_one = 0;
		//�ҵ���Ӧ�е��ļ�ƫ��
		do
		{
			for(record_cnt=0;record_cnt<30;record_cnt++)
			{
				if(Gcode_current_position[record_cnt].Gcode_LineNumb == (RePrintData.record_line))
				{
						RePrintData.offset = Gcode_current_position[record_cnt].Gcode_fileOffset;
						break;
				}
			}
		}while(record_cnt == 30 && RePrintData.record_line--);
		
		f_lseek(srcfp, RePrintData.offset);

		initFIFO(&gcodeTxFIFO);
		initFIFO(&gcodeCmdTxFIFO);
		initFIFO(&gcodeCmdRxFIFO);
		
		memset(udiskBuffer.buffer[0],'\n',sizeof(udiskBuffer.buffer[0]));
		memset(udiskBuffer.buffer[1],'\n',sizeof(udiskBuffer.buffer[1]));
		udiskBuffer.current = 0;
		udiskBuffer.p = udiskBuffer.buffer[udiskBuffer.current];
		udiskBuffer.state[udiskBuffer.current] = udisk_buf_full;
		udiskBuffer.state[(udiskBuffer.current+1)%2] = udisk_buf_empty;

		note_flag = 1;
		gcodeLineCnt = 0;
		RePrintData.record_line = 0;
		
		//��ָ��
		while(checkFIFO(&gcodeTxFIFO)!= fifo_full)
		{
			udiskFileR(srcfp);												
			pushTxGcode();	
			/*last_tick=getTick();
			if(getTickDiff(last_tick, getTick()) > 4000)
				break;	*/
		}	
		//��N-1 M110*15
		printerInit();
	}	
}

void pushTxGcode(void)		//��udiskBuffer��������ȡ����Ч��gcodeָ��������кţ�push��gcodeTxFIFO
{

	int8_t *Ztemp;
	int8_t *Ftemp;
	unsigned char i=0;
	
	static unsigned char position_cnt = 0;
	unsigned char numb_cnt = 0;
	
	unsigned char gcode[FIFO_SIZE];		//�洢��udiskBuffer��ȡ��һ��gcode
	unsigned char *p=gcode;				//ָ��gcode��ָ��
	unsigned char gcode_tx[FIFO_SIZE]={0};	//�ɷ��͵�gcodeָ������кź�У����
	unsigned char *p_tx=gcode_tx;		//ָ��gcode_tx��ָ��
	unsigned long gcodeLineCnt_b;		//�ݴ�gcodeLineCnt
	unsigned char lineCntBuf[20];		//�洢�к��ַ���
	unsigned char *p_cnt=lineCntBuf;	
	unsigned char checkSum=0;			//У���
	unsigned char ulockCnt=0;			//��ע�� ��������ֹ ���������ݣ����²��ܴ�udisk��ȡ�ļ����������

	if(checkFIFO(&gcodeTxFIFO)== fifo_full)			//������
		return;

	if(udiskBuffer.state[udiskBuffer.current] == udisk_buf_empty)	//buffer��
		return;

			while(*udiskBuffer.p != '\n'  && *udiskBuffer.p != '\r')	//�н���
			{
				if(p-gcode > (FIFO_SIZE-10))	//һ��ָ��̫������������ע�͵������ַ�
				{
					*(udiskBuffer.p +1)= ';';
					break;
				}

				//if(ulockCnt++ > FIFO_SIZE && p == gcode)		//��ֹ��ע�� �������
				//{
				//	return;
				//}

				

				if(*udiskBuffer.p == ';')	//ȥ�� ';' �����ע��
					note_flag =  0;

				if(note_flag)
					*p++ = *udiskBuffer.p++;	//��ȡ��Чgcodeָ��
				else
					udiskBuffer.p++;

				if(udiskBuffer.p == udiskBuffer.buffer[udiskBuffer.current]+ UDISKBUFLEN)	//��ǰbuffer ��ȡ����,ת������һbuffer
				{
					memset(udiskBuffer.buffer[udiskBuffer.current],'\n',sizeof(udiskBuffer.buffer[0]));		//buffer ������'\n'
					udiskBuffer.state[udiskBuffer.current] = udisk_buf_empty;								//buffer ״̬��empty
					udiskBuffer.current = (udiskBuffer.current+1)%2;										//ת��һ��buffer
					udiskBuffer.p = udiskBuffer.buffer[udiskBuffer.current];								//��ַָ����һ��buffer
				}

				if(ulockCnt++ > FIFO_SIZE && p == gcode)		//��ֹ��ע�� �������
				{
					return;
				}


			}
			udiskBuffer.p++;	//����'\n'�ַ�
			if(udiskBuffer.p == udiskBuffer.buffer[udiskBuffer.current]+ UDISKBUFLEN)	//��ǰbuffer ��ȡ����,ת������һbuffer
				{
					memset(udiskBuffer.buffer[udiskBuffer.current],'\n',sizeof(udiskBuffer.buffer[0]));		//buffer ������'\n'
					udiskBuffer.state[udiskBuffer.current] = udisk_buf_empty;								//buffer ״̬��empty
					udiskBuffer.current = (udiskBuffer.current+1)%2;										//ת��һ��buffer
					udiskBuffer.p = udiskBuffer.buffer[udiskBuffer.current];								//��ַָ����һ��buffer
				}

			note_flag = 1;		

			if(p > gcode)		//��ȡ����gcodeָ��
			{
				while(*(--p) == 32);	//ȥ��gcodeָ������Ŀո�
					p++;
				
				*p_tx++ = 'N';					//��'N'	
				
				gcodeLineCnt_b = gcodeLineCnt;			//���к�
				
			
				*p_cnt++=gcodeLineCnt_b%10 + 48;
				gcodeLineCnt_b /= 10;
				while(gcodeLineCnt_b!=0)
				{
					*p_cnt++=gcodeLineCnt_b%10 + 48;
					gcodeLineCnt_b /= 10;
				}


				while(p_cnt>lineCntBuf)
					*p_tx++ = *--p_cnt;
				
				*p_tx++ = 32;							//�ӿո�

				gcodeLineCnt++;
				//��˫��ͷ�����ж�
				if((gcode[0]=='T')&&(gcode[1]=='0'))
				{
					RePrintData.spayerchoose = 0;
					//chen 11.8 ����һ��
					gCfgItems.sd_saving = 1;
				}
				if((gcode[0]=='T')&&(gcode[1]=='1'))
				{
					RePrintData.spayerchoose = 1;
					//chen 11.8 ����һ��
					gCfgItems.sd_saving = 1;
				}
				//
				getTargetTemp(&gcode[0],p);			//��ȡĿ���¶�
				getFanStatus(&gcode[0],p);				//��ȡ����״̬
				
				p_cnt=gcode;								//��gcodeָ��,��ʱʹ��p_cnt
				while(p_cnt<p)								
				{
				*p_tx++ = *p_cnt++;
				}
				*p_tx++ = '*';										//��'*'

															//��У��
				p_cnt= gcode_tx;
				while(*p_cnt != '*')
					checkSum ^= *p_cnt++;
				
				if(checkSum/100 != 0)				
				{
					*p_tx++ = checkSum/100 + 48;
					*p_tx++ = (checkSum/10)%10 + 48;
					*p_tx++ = checkSum%10 + 48;
				}
				else if(checkSum/10 != 0)
				{
					*p_tx++ = checkSum/10 + 48;
					*p_tx++ = checkSum%10 + 48;
				}
				else
					*p_tx++ = checkSum%10 + 48;
				
				*p_tx++ = '\n';								//��'\n'

				//USART2_CR1 &= 0xff9f;
				pushFIFO(&gcodeTxFIFO,&gcode_tx[0]);			//�����

				Ztemp = (int8_t *)strstr((const char *)&gcode_tx[0],(const char *)"Z");
				if(Ztemp)
				{
					i=0;
					strcpy((char*)gCfgItems.z_display_pos_bak,(char*)gCfgItems.z_display_pos);
					memset((char *)gCfgItems.z_display_pos,0,sizeof(gCfgItems.z_display_pos));
					while((*(Ztemp+1+i)!=' ')&&(*(Ztemp+1+i)!='*')&&(*(Ztemp+1+i)!='\r')&&(*(Ztemp+1+i)!='\n'))
					{
					#ifdef SAVE_FROM_SD
						gCfgItems.sd_saving = 1;
						
						gCfgItems.sd_save_flg = 1;
						//gcodeLineCntBak1 = gcodeLineCnt;
					#endif
						
					
						if((*(Ztemp+1+i)=='+')||(*(Ztemp+1+i)=='-'))
						{
							i++;
							continue;
						}
						else
						{
							gCfgItems.z_display_pos[i] = *(Ztemp+1+i);
							//i++;
						}
						
						if(((gCfgItems.z_display_pos[i]>='a')&&(gCfgItems.z_display_pos[i]<='z'))
							||((gCfgItems.z_display_pos[i]>='A')&&(gCfgItems.z_display_pos[i]<='Z')))
						{
							strcpy((char*)gCfgItems.z_display_pos,(char*)gCfgItems.z_display_pos_bak);
							break;
						}
						
						i++;
						
						if(i>20)break;
					}
				}

				Ftemp = (int8_t *)strstr((const char *)&gcode_tx[0],(const char *)"F");
				if(Ftemp)
				{
					i=0;
					while((*(Ftemp+i)!=' ')&&((*(Ftemp+i)!='\r'))&&((*(Ftemp+i)!='\n'))&&((*(Ftemp+i)!='*')))
					{
						//�ٶ�Fxxxxx\n
						gCfgItems.F_speed[i] = *(Ftemp+i);
						i++;	

						if(i>20)break;
					}

					gCfgItems.F_speed[i] = '\n';
				}



				RePrintData.offset =  f_tell(srcfp)-UDISKBUFLEN;
				if(udiskBuffer.state[(udiskBuffer.current+1)%2] == udisk_buf_full)
					RePrintData.offset -= UDISKBUFLEN;
				RePrintData.offset += udiskBuffer.p - udiskBuffer.buffer[udiskBuffer.current];
				//USART2_CR1 |= 0x0060;

				//20151012
				Gcode_current_position[position_cnt].Gcode_LineNumb= gcodeLineCnt;
				Gcode_current_position[position_cnt++].Gcode_fileOffset= RePrintData.offset;
				if(position_cnt >= 30)
				{
					position_cnt = 0;
				}
				

			}
}

#ifdef SAVE_FROM_SD
FIL pfs;

void sd_saved_data()
{
	char string[20];
	//mksReprint.card.openFile(MKS_PFT_NAME, false);
	//if (mksReprint.card.isFileOpen()) 
	char name[30]={0};
	if((gCfgItems.sd_saving)&&(printerStaus == pr_working))
	{
		if(gCfgItems.fileSysType == FILE_SYS_SD)
			strcpy((char *)name, "1:");
		else
			strcpy((char *)name, "0:");

		strcat(name,(char *)path_reprint);

		if (f_open(&pfs, (const TCHAR *)name, FA_OPEN_ALWAYS | FA_READ | FA_WRITE) == FR_OK)
		{
			memset(string,0,sizeof(string));
			strcpy(string,"start\n");
			f_printf(&pfs,string);

			//�����ļ�λ��
			memset(string,0,sizeof(string));
			sprintf(string,"%d",RePrintData.offset);
			strcat(string,"\n");
			f_printf(&pfs,string);
			//�����ӡͷ�¶�
			memset(string,0,sizeof(string));
			sprintf(string,"%d",(int)gCfgItems.desireSprayerTemp[0]);
			strcat(string,"\n");
			f_printf(&pfs,string);
			
			memset(string,0,sizeof(string));
			sprintf(string,"%d",(int)gCfgItems.desireSprayerTemp[1]);
			strcat(string,"\n");
			f_printf(&pfs,string);		
			
			//�����ȴ��¶�
			memset(string,0,sizeof(string));
			sprintf(string,"%d",(int)gCfgItems.desireBedTemp);
			strcat(string,"\n");
			f_printf(&pfs,string);
			
			//������Ⱥ�ʱ��
			//��ӡʱ��:hour
			memset(string,0,sizeof(string));	
			sprintf(string,"%d",print_time.hours);
			strcat(string,"\n");
			f_printf(&pfs,string);
			
			//��ӡʱ��:min
			memset(string,0,sizeof(string));				
			sprintf(string,"%d",print_time.minutes);
			strcat(string,"\n");
			f_printf(&pfs,string);
				
			//�������
			memset(string,0,sizeof(string));
			sprintf(string,"%d",gCfgItems.fanOnoff);
			strcat(string,"\n");
			f_printf(&pfs,string);
			
			memset(string,0,sizeof(string));					
			sprintf(string,"%d",gCfgItems.fanSpeed);
			strcat(string,"\n");
			f_printf(&pfs,string);			
			//����Z��߶�
			memset(string,0,sizeof(string));
			sprintf(string,"%s",gCfgItems.z_display_pos);
			strcat(string,"\n");
			f_printf(&pfs,string);
			
			//���浱ǰ����ͷ
			memset(string,0,sizeof(string));
			//chen 11.8
			sprintf(string,"%d",RePrintData.spayerchoose);
			//sprintf(string,"%d",gCfgItems.curSprayerChoose);
			strcat(string,"\n");
			f_printf(&pfs,string);

			memset(string,0,sizeof(string));
			strcpy(string,"end");						//"end"	
			strcat(string,"\n");
			f_printf(&pfs,string);
			
			f_close(&pfs);
		}
		
		gCfgItems.sd_saving = 0;	
	}
}
#if 0
void sd_saved_data()
{
	unsigned char sd_buf_w[100];
	//FIL fp_reprint_w;
	unsigned int bw_repint;
	unsigned char i=0;
	if((gCfgItems.sd_saving)&&(printerStaus == pr_working))
	{
		memset(sd_buf_w,0,sizeof(sd_buf_w));
		sprintf((char *)sd_buf_w,"P:%d|T0:%.2f|T1:%.2f|B:%.2f|FanOn:%d|FanSp:%d|h:%d|m:%d|Z:%s|C:%d|",\
			RePrintData.offset,gCfgItems.desireSprayerTemp[0],gCfgItems.desireSprayerTemp[1],\
			gCfgItems.desireBedTemp,gCfgItems.fanOnoff,gCfgItems.fanSpeed,\
			print_time.hours,print_time.minutes,gCfgItems.z_display_pos,gCfgItems.curSprayerChoose);
		//strcat(sd_buf_w,gCfgItems.z_display_pos);
		//sd_buf_w��ÿ��ֵ����0x4D���(0x4D��Ӧ��ASCII��M)
		
		for(i=0;i<strlen((const char *)sd_buf_w);i++)
		{
			sd_buf_w[i] = sd_buf_w[i]^0x4d; 
		}
		
		if(gCfgItems.fileSysType == FILE_SYS_SD)
		{
			//FATFS_LinkDriver_sd(&SD_Driver, SD_Path);
			//f_mount(&fs, (TCHAR const*)SD_Path, 0); 	//
			strcpy((char *)path_bak, "1:");
		}
		else
		{
			//FATFS_LinkDriver_usb(&SD_Driver, USBH_Path);
			//f_mount(&fs, (TCHAR const*)USBH_Path, 0); 	//		
			strcpy((char *)path_bak, "0:");
		}


			strcat((char *)path_bak,(const char *)path_reprint);
			if(f_open(&fp_reprint_rw, (const char *)path_bak, FA_WRITE|FA_OPEN_ALWAYS)== FR_OK)
			{
				f_write(&fp_reprint_rw, sd_buf_w, 100,&bw_repint);
				f_close(&fp_reprint_rw);	
				gCfgItems.sd_saving = 0;
			}
		}
}
#endif
uint32_t pfspos;

int16_t pft_get() 
{
	BYTE readByte;
	UINT rc;
	if (f_read(&pfs, &readByte, 1, &rc) != FR_OK)				   
	{
		readByte = -1;
	}
	else
	{
		pfspos += rc;
	}
	return (int16_t) readByte;
};

bool mks_get_commands() {
	char string[20];
	char commands_count = 0;
  	uint16_t sd_count = 0;
	bool end_ok = false;
  
  while(pfspos < pfs.fsize)
  {
	const int16_t n = pft_get();
	if(n == -1)	break;
	char sd_char = (char)n;

	if ( sd_char == '\n') 	//"start\r\n"
	{
		string[sd_count-1] = 0;		//ȥ�� "\r"
		switch(commands_count)
		{
			case 0:			//start
				break;
			case 1:			//��ȡ�ļ�λ��
				RePrintData.offset = atoi(string);
				rePrintOffset = RePrintData.offset;
				break;
			case 2:			//��ȡ��ӡͷ�¶�
				gCfgItems.desireSprayerTemp[0] = atoi(string);
				break;
			case 3:			//��ȡ��ӡͷ�¶�
				gCfgItems.desireSprayerTemp[1] = atoi(string);
				break;				
			case 4:			//��ȡ�ȴ��¶�
				gCfgItems.desireBedTemp = atoi(string);
				break;
			case 5:			//hour
				print_time.hours= atoi(string);
				break;
			case 6:			//min
				print_time.minutes= atoi(string);
				break;
			case 7:			//��ȡ���ȿ���
				gCfgItems.fanOnoff = atoi(string);
				break;
			case 8:		//��ȡ���ȷ���
				gCfgItems.fanSpeed = atoi(string);
				break;
			case 9:		//��ȡ���ȷ���
				strcpy((char *)gCfgItems.z_display_pos,string);
				break;				
			case 10:		//��ȡ��ǰ����ͷ
				gCfgItems.curSprayerChoose = atoi(string);
				break;				
			case 11:		//end
				if(strcmp(string,"end") == 0)
					end_ok = true;
				break;
			default: break;				
				
			}

	  	sd_count = 0;
		memset(string,0,sizeof(string));
		commands_count++;
	}
	else 
	{
		  string[sd_count++] = sd_char;
	}
		
  }
  return(end_ok);
}

extern FATFS fs;
extern __IO uint32_t delaycnt1;
extern USBH_HandleTypeDef hUsbHostFS;

void sd_data_recover()
{
	bool get_ok = false;
    char name[30]={0};

	if(gCfgItems.fileSysType == FILE_SYS_SD)
	{
		FATFS_LinkDriver_sd(&SD_Driver, SD_Path);
		f_mount(&fs, (TCHAR const*)SD_Path, 0);     //skyblue 2016-12-13  //**
		strcpy((char *)path_bak, "1:");
	}
	else
	{			
		FATFS_LinkDriver_usb(&USBH_Driver, USBH_Path);
		f_mount(&fs, (TCHAR const*)USBH_Path, 0);     //skyblue 2016-12-13  //**
		strcpy((char *)path_bak, "0:");
	}
	
	strcat(name,(char *)path_bak);
    strcat(name,(const char *)path_reprint);    
	
	if (f_open(&pfs, (const TCHAR *)name, FA_OPEN_EXISTING | FA_READ) == FR_OK)		
	{
		pfspos = 0;
		get_ok = mks_get_commands();
		f_close(&pfs);
		if(get_ok)
		{
			gCfgItems.rePrintFlag = printer_pwdwn_reprint;
			RePrintData.printerStatus = printer_pwdwn_reprint;
		}
	}
	
	return;//(get_ok);

}

void mks_clearFile()
{
	char string[20]="clear";
	char name[30]={0};
	
	if(gCfgItems.fileSysType == FILE_SYS_SD)
		strcpy((char *)name, "1:");
	else
		strcpy((char *)name, "0:");
	
	strcat(name,(char *)path_reprint);
	
	if (f_open(&pfs, (const TCHAR *)name, FA_CREATE_ALWAYS |  FA_WRITE) == FR_OK)
    {
       f_printf(&pfs,string);
       f_close(&pfs);
    }
}
#if 0
extern FATFS fs; 

void sd_data_recover()
{
	const char *sdstr_temp;
	unsigned char i=0;
	char sdread_temp[20]={0};
	//FIL fp_reprint_r;
	unsigned char sd_buf_r[100];
	unsigned int br_repint;
	
	if(gCfgItems.fileSysType == FILE_SYS_SD)
	{
		//**f_mount(1, (const char *)&fs);
		FATFS_LinkDriver_sd(&SD_Driver, SD_Path);
		f_mount(&fs, (TCHAR const*)SD_Path, 0);     //skyblue 2016-12-13  //**
		strcpy((char *)path_bak, "1:");
	}
	else
	{
		FATFS_LinkDriver_usb(&SD_Driver, USBH_Path);
		f_mount(&fs, (TCHAR const*)USBH_Path, 0);     //skyblue 2016-12-13  //**
		strcpy((char *)path_bak, "0:");

	}
		

	strcat((char *)path_bak,(const char *)path_reprint);	
	if(f_open(&fp_reprint_rw, (const char *)path_bak, FA_READ)== FR_OK)
	{
		memset(sd_buf_r,0,sizeof(sd_buf_r));
		f_read(&fp_reprint_rw, sd_buf_r, 100, &br_repint);
	//sd_buf_r��ÿ��ֵ����0x4D���(0x4D��Ӧ��ASCII��M)
		
		for(i=0;i<strlen((const char *)sd_buf_r);i++)
		{
			sd_buf_r[i] = sd_buf_r[i]^0x4d; 
		}
		
		sdstr_temp = strstr((const char *)sd_buf_r,(const char *)"P:");
		if(sdstr_temp)
		{
			i=0;
			while(*(sdstr_temp+2+i)!='|')
			{
				sdread_temp[i] = *(sdstr_temp+2+i);
				i++;
				if(i>20)break;
			}
			sdread_temp[i] = 0;
			RePrintData.offset = atoi(sdread_temp);
			rePrintOffset = RePrintData.offset;
		}
		
		sdstr_temp = strstr((const char *)sd_buf_r,(const char *)"T0:");
		if(sdstr_temp)
		{
			i=0;
			while(*(sdstr_temp+3+i)!='|')
			{
				sdread_temp[i] = *(sdstr_temp+3+i);
				i++;
				if(i>20)break;
			}
			sdread_temp[i] = 0;
			gCfgItems.desireSprayerTemp[0] = atoi(sdread_temp);
		}	
		sdstr_temp = strstr((const char *)sd_buf_r,(const char *)"T1:");
		if(sdstr_temp)
		{
			i=0;
			while(*(sdstr_temp+3+i)!='|')
			{
				sdread_temp[i] = *(sdstr_temp+3+i);
				i++;
				if(i>20)break;
			}
			sdread_temp[i] = 0;
			gCfgItems.desireSprayerTemp[1] = atoi(sdread_temp);
		}	
		sdstr_temp = strstr((const char *)sd_buf_r,(const char *)"B:");
		if(sdstr_temp)
		{
			i=0;
			while(*(sdstr_temp+2+i)!='|')
			{
				sdread_temp[i] = *(sdstr_temp+2+i);
				i++;
				if(i>20)break;
			}
			sdread_temp[i] = 0;
			gCfgItems.desireBedTemp = atoi(sdread_temp);
		}	
		sdstr_temp = strstr((const char *)sd_buf_r,(const char *)"FanSp:");
		if(sdstr_temp)
		{
			i=0;
			while(*(sdstr_temp+6+i)!='|')
			{
				sdread_temp[i] = *(sdstr_temp+6+i);
				i++;
				if(i>20)break;
			}
			sdread_temp[i] = 0;
			gCfgItems.fanSpeed = atoi(sdread_temp);
		}
		sdstr_temp = strstr((const char *)sd_buf_r,(const char *)"FanOn:");
		if(sdstr_temp)
		{
			i=0;
			while(*(sdstr_temp+6+i)!='|')
			{
				sdread_temp[i] = *(sdstr_temp+6+i);
				i++;
				if(i>20)break;
			}
			sdread_temp[i] = 0;
			gCfgItems.fanOnoff = atoi(sdread_temp);
		}		
		sdstr_temp = strstr((const char *)sd_buf_r,(const char *)"h:");
		if(sdstr_temp)
		{
			i=0;
			while(*(sdstr_temp+2+i)!='|')
			{
				sdread_temp[i] = *(sdstr_temp+2+i);
				i++;
				if(i>20)break;
			}
			sdread_temp[i] = 0;
			print_time.hours = atoi(sdread_temp);
		}
		sdstr_temp = strstr((const char *)sd_buf_r,(const char *)"m:");
		if(sdstr_temp)
		{
			i=0;
			while(*(sdstr_temp+2+i)!='|')
			{
				sdread_temp[i] = *(sdstr_temp+2+i);
				i++;
				if(i>20)break;
			}
			sdread_temp[i] = 0;
			print_time.minutes= atoi(sdread_temp);
		}
		sdstr_temp = strstr((const char *)sd_buf_r,(const char *)"Z:");
		if(sdstr_temp)
		{
			i=0;
			while(*(sdstr_temp+2+i)!='|')
			{
				gCfgItems.z_display_pos[i] = *(sdstr_temp+2+i);
				i++;
				if(i>20)break;
			}
			gCfgItems.z_display_pos[i] = 0;
		}
		sdstr_temp = strstr((const char *)sd_buf_r,(const char *)"C:");
		if(sdstr_temp)
		{
			i=0;
			while(*(sdstr_temp+2+i)!='|')
			{
				sdread_temp[i] = *(sdstr_temp+2+i);
				i++;
				if(i>20)break;
			}
			sdread_temp[i] = 0;
			gCfgItems.curSprayerChoose = atoi(sdread_temp);
		}		
/*
		sdstr_temp = strstr(sd_buf,"File:");
		if(sdstr_temp)
		{
			i=0;
			while(*(sdstr_temp+2+i)!='\n')
			{
				sdread_temp[i] = *(sdstr_temp+2+i);
				i++;
				if(i>100)break;
			}
			sdread_temp[i] = 0;
			strcpy(sdread_temp,curFileName);
		}			
*/
		f_close(&fp_reprint_rw);

		gCfgItems.rePrintFlag = printer_pwdwn_reprint;
		RePrintData.printerStatus = printer_pwdwn_reprint;
	}
	

}
#endif

#endif


