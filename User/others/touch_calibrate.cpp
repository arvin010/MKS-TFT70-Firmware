#include "stdint.h"

#include "mks_touch_screen.h"

//#include "stm3210c_eval_lcd.h"
#include "draw_ui.h"
//#include "printer.h"
//#include "time_tick.h"
#include "tim.h"        //skyblue 2016-12-13
#include "mks_cfg.h"
#include "touch_calibrate.h"
#include "lcdConf.h"
#include "GUITouchConf.h"
#include "mks_cfg.h"



#define LANG_ENGLISH 3

#if defined(__cplusplus)
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif
#ifndef GUI_FLASH 
	#define GUI_FLASH 
#endif   
extern GUI_FLASH const GUI_FONT GUI_FontHZ_fontHz14;
extern int  GUI_TOUCH_X_MeasureState(void);
extern CFG_ITMES gCfgItems;

#if defined(__cplusplus)
}
#endif
  
  
//��һ�������� 
//����У׼�õ� 
void Drow_Touch_Point(uint16_t x,uint16_t y) 
{ 
	#if defined(TFT70)
	GUI_DrawHLine(y,x-36,x+37);//���� 
	GUI_DrawVLine(x,y-36,y+37);//���� 
	GUI_DrawPoint(x+1,y+1); 
	GUI_DrawPoint(x-1,y+1); 
	GUI_DrawPoint(x+1,y-1); 
	GUI_DrawPoint(x-1,y-1); 
	GUI_DrawCircle(x,y,18);//������Ȧ 	
	#elif defined(TFT35)
	GUI_DrawHLine(y,x-12,x+13);//���� 
	GUI_DrawVLine(x,y-12,y+13);//���� 
	GUI_DrawPoint(x+1,y+1); 
	GUI_DrawPoint(x-1,y+1); 
	GUI_DrawPoint(x+1,y-1); 
	GUI_DrawPoint(x-1,y-1); 
	GUI_DrawCircle(x,y,6);//������Ȧ 					
	#endif
	

	
	if(gCfgItems.language == LANG_COMPLEX_CHINESE)
	{
		//GUI_DispStringAt(creat_title_text(), 0, 0);
		GUI_DispStringAt("Ո�c�������c", 100, 85);
	}
	else if(gCfgItems.language == LANG_SIMPLE_CHINESE)
	{
		//GUI_DispStringAt(creat_title_text(), 0, 0);
		GUI_DispStringAt("�������ĵ�", 100, 85);
	}
	else
	{
		//GUI_DispStringAt("Touch Calibrate", 0, 0);
		GUI_DispStringAt("Please click the ring", 80, 85);
	}
 
}  

static int32_t touch_time = 0;
#if 1    

	#if 0
	void Touch_Adjust()
	{
		signed short  pos_temp[5][2];//���껺��ֵ 
		signed short xMin, xMax, yMin, yMax;
		uint8_t cnt=0;  
		uint32_t d1,d2; 
		uint32_t tem1,tem2; 
		float fac; 
		uint16_t x, y, tmpCood;
		float xfac, yfac, xoff, yoff;
		int i;
		int8_t buf[30];
		int32_t tmpTime, diffTime;
	    static int8_t press_flag = 0;
		
		
		cnt=0; 

		//SZ_STM32_LCDInit();
		//SZ_TS_Init();
		
		GUI_SetBkColor(GUI_BLUE); 
		GUI_SetColor(GUI_WHITE);
		GUI_Clear();
		if(gCfgItems.language == LANG_COMPLEX_CHINESE)
		{
			GUI_SetFont(&GUI_FontHZ16);
		}
		else
		{
			GUI_SetFont(&FONT_TITLE);
		}


		
		/*LCD_SetBackColor(LCD_COLOR_WHITE);
		LCD_SetTextColor(LCD_COLOR_BLUE);
		LCD_Clear(LCD_COLOR_WHITE);*/
	 
		Drow_Touch_Point(15,15);//���� 1 
		while(1) 
		{ 
			Commands::checkForPeriodicalActions(true);                      //modify skyblue
		//	if(Pen_Point.Key_Sta==Key_Down)//���������� 
			{ 
				while(GUI_TOUCH_X_MeasureState() != PEN_DOWN)
				{
					Commands::checkForPeriodicalActions(true);      ////modify skyblue
				}
				
				x = GUI_TOUCH_X_MeasureX();
				y = GUI_TOUCH_X_MeasureY();
	                        
	            while(GUI_TOUCH_X_MeasureState() != PEN_UP)
	            {
	            	Commands::checkForPeriodicalActions(true);                      ////modify skyblue
				}

				if ((x <= 50) ||  (y <= 50) ||(x >= 4060)  ||(y >= 4060) )
			    {
			    	continue;
			    }

				/* �ӳ�ȥ�� */
	           /* if(press_flag)
	            {
	              tmpTime = getTick();;
	              
	              diffTime = getTickDiff(tmpTime, touch_time);

	              if(diffTime < 500)
	              {
	                      continue;
	              }
	              
	            }
	            touch_time = getTick();
	            press_flag = 1;*/

				#if GUI_TOUCH_SWAP_XY   /* Is X/Y swapped ? */		   
					tmpCood = x;
					x = y;
					y = tmpCood;
				#endif
				
	            pos_temp[cnt][0] = x; 
				pos_temp[cnt][1] = y;
	                        
				
				cnt++; 
				 
				switch(cnt) 
				{ 
					case 1: 
						GUI_Clear();//���� 
						Drow_Touch_Point(785, 15);//����2 				
					break; 

					case 2: 
						GUI_Clear();//���� 
						Drow_Touch_Point(15, 465);//����3 
					break; 

					case 3: 
						GUI_Clear();//���� 
						Drow_Touch_Point(785, 465);//����4 
					break; 

					case 4: 
						GUI_Clear();//���� 
						Drow_Touch_Point(400, 240);//����5
					break; 

					case 5:
						//�Ա���� 
						tem1=abs(pos_temp[0][0]-pos_temp[1][0]);//x1-x2 
						tem2=abs(pos_temp[0][1]-pos_temp[1][1]);//y1-y2 
						tem1*=tem1; 
						tem2*=tem2; 
						d1=(tem1+tem2);//�õ�1,2�ľ��� 
						tem1=abs(pos_temp[2][0]-pos_temp[3][0]);//x3-x4 
						tem2=abs(pos_temp[2][1]-pos_temp[3][1]);//y3-y4 
						tem1*=tem1; 
						tem2*=tem2; 
						d2=(tem1+tem2);//�õ�3,4�ľ���
						
						fac=(float)((float)d1/(float)d2); 
					
						if(fac<0.7||fac>1.4||d1==0||d2==0)//���ϸ� 
						{ 
							cnt=0; 
							GUI_Clear();//���� 
							Drow_Touch_Point(15,15); 
							continue; 
						} 
						tem1=abs(pos_temp[0][0]-pos_temp[2][0]);//x1-x3 
						tem2=abs(pos_temp[0][1]-pos_temp[2][1]);//y1-y3 
						tem1*=tem1; 
						tem2*=tem2; 
						d1=(tem1+tem2);//�õ�1,3�ľ��� 
						tem1=abs(pos_temp[1][0]-pos_temp[3][0]);//x2-x4 
						tem2=abs(pos_temp[1][1]-pos_temp[3][1]);//y2-y4 
						tem1*=tem1; 
						tem2*=tem2; 
						d2=(tem1+tem2);//�õ�2,4�ľ��� 
						fac=(float)((float)d1/(float)d2); 
						
						if(fac<0.7||fac>1.4)//���ϸ� 
						{ 
							cnt=0; 
							GUI_Clear();//���� 
							Drow_Touch_Point(15,15); 
							continue; 
						}//��ȷ�� 
						//�Խ������ 
						tem1=abs(pos_temp[1][0]-pos_temp[2][0]);//x1-x3 
						tem2=abs(pos_temp[1][1]-pos_temp[2][1]);//y1-y3 
						tem1*=tem1; 
						tem2*=tem2; 
						d1=(tem1+tem2);//�õ�1,4�ľ��� 
						
						tem1=abs(pos_temp[0][0]-pos_temp[3][0]);//x2-x4 
						tem2=abs(pos_temp[0][1]-pos_temp[3][1]);//y2-y4 
						tem1*=tem1; 
						tem2*=tem2; 
						d2=(tem1+tem2);//�õ�2,3�ľ��� 
						fac=(float)((float)d1/(float)d2); 

						
						if(fac<0.7||fac>1.4)//���ϸ� 
						{ 
							cnt=0; 
							GUI_Clear();//���� 
							Drow_Touch_Point(15,15); 
							continue; 
						}

						//�е�λ���ж�					
						tem1=abs(pos_temp[4][0]-pos_temp[0][0]);//x5-x1
						tem2=abs(pos_temp[4][0]-pos_temp[3][0]);//x5-x4 

						fac=(float)((float)tem1/(float)tem2); 
						
						if(fac<0.6||fac>1.4)//���ϸ� 
						{ 
							cnt=0; 
							GUI_Clear();//���� 
							Drow_Touch_Point(15,15); 
							continue; 
						}

						tem1=abs(pos_temp[4][1]-pos_temp[0][1]);//y5-y1
						tem2=abs(pos_temp[4][1]-pos_temp[3][1]);//y5-y4 

						fac=(float)((float)tem1/(float)tem2); 
						
						if(fac<0.6||fac>1.4)//���ϸ� 
						{ 
							cnt=0; 
							GUI_Clear();//���� 
							Drow_Touch_Point(15,15); 
							continue; 
						}

						tem1=abs(pos_temp[4][0]-pos_temp[1][0]);//x5-x2
						tem2=abs(pos_temp[4][0]-pos_temp[2][0]);//x5-x3

						fac=(float)((float)tem1/(float)tem2); 
						
						if(fac<0.6||fac>1.4)//���ϸ� 
						{ 
							cnt=0; 
							GUI_Clear();//���� 
							Drow_Touch_Point(15,15); 
							continue; 
						}

						tem1=abs(pos_temp[4][1]-pos_temp[1][1]);//y5-y2
						tem2=abs(pos_temp[4][1]-pos_temp[2][1]);//y5-y3

						fac=(float)((float)tem1/(float)tem2); 
						
						if(fac<0.6||fac>1.4)//���ϸ� 
						{ 
							cnt=0; 
							GUI_Clear();//���� 
							Drow_Touch_Point(15,15); 
							continue; 
						}
						/*
						if(LCD_GetSwapXYEx(0))
	                    {
	                      GUI_TOUCH_Calibrate(GUI_COORD_X, 15, 225, pos_temp[0][0], pos_temp[3][0]);
	                      GUI_TOUCH_Calibrate(GUI_COORD_Y, 15, 305, pos_temp[0][1], pos_temp[3][1]);
	                    } 
	                    else
	                    {
	                      GUI_TOUCH_Calibrate(GUI_COORD_X, 15, 305, pos_temp[0][0], pos_temp[3][0]);
	                      GUI_TOUCH_Calibrate(GUI_COORD_Y, 15, 225, pos_temp[0][1], pos_temp[3][1]);
	                   	}
						*/
						xMin = pos_temp[0][0] - ( pos_temp[1][0]-pos_temp[0][0]) * (0 + 15) / 770;
	  					xMax = pos_temp[0][0] + ( pos_temp[1][0]-pos_temp[0][0]) * (LCD_XSIZE - 15) / 770;
	  					yMin = pos_temp[0][1] - ( pos_temp[2][1]-pos_temp[0][1]) * (0 + 15) / 450;
	  					yMax = pos_temp[0][1] + ( pos_temp[2][1]-pos_temp[0][1]) * (LCD_YSIZE - 15) / 450;

						gCfgItems.touch_adj_xMin = xMin;
						gCfgItems.touch_adj_xMax = xMax;
						gCfgItems.touch_adj_yMin = yMin;
						gCfgItems.touch_adj_yMax = yMax;

						setTouchBound(xMin, xMax, yMax, yMin);
						
						GUI_Clear();//���� 
						
						if(gCfgItems.language == LANG_ENGLISH)
						{
							GUI_DispStringAt("Touch Screen Adjust OK!", 60, 100);//У����� 
						}
						else if(LANG_COMPLEX_CHINESE)
						{
							GUI_DispStringAt("У�����!", 100, 100);
						}					
						else
						{
							GUI_DispStringAt("У�����!", 100, 100);
						}
						for(i = 0; i < 65535 * 100; i++);

						GUI_SetBkColor(gCfgItems.background_color);
											
						GUI_Clear();//���� 

						draw_return_ui();

						return;//У����� 
				} 
			} 
		}
	}

	#endif

void Touch_Adjust()
{
	signed short  pos_temp[5][2];//���껺��ֵ 
	
	uint8_t cnt=0;  
	uint32_t d1,d2; 
	uint32_t tem1,tem2; 
	float fac; 
	uint16_t x, y, tmpCood;
	float xfac, yfac, xoff, yoff;
	int i;
	int8_t buf[30];
	int32_t tmpTime, diffTime;
	signed short xMin_temp[4], xMax_temp[4], yMin_temp[4], yMax_temp[4];
	signed short xMin, xMax, yMin, yMax;
	
	cnt=0; 

	//SZ_STM32_LCDInit();
	//SZ_TS_Init();
	
	GUI_SetBkColor(GUI_BLUE); 
	GUI_SetColor(GUI_WHITE);
	//GUI_SetFont(&GUI_FontHZ_fontHz14);
	GUI_Clear();
	if(gCfgItems.language == LANG_COMPLEX_CHINESE || gCfgItems.language == LANG_SIMPLE_CHINESE)
	{
		GUI_SetFont(&GUI_FontHZ16);
		
		GUI_UC_SetEncodeNone();
	}
	else
	{
		GUI_SetFont(&FONT_TITLE);
	}	

	
	/*LCD_SetBackColor(LCD_COLOR_WHITE);
	LCD_SetTextColor(LCD_COLOR_BLUE);
	LCD_Clear(LCD_COLOR_WHITE);*/
 
	
	#if defined(TFT70)
	Drow_Touch_Point(50,50);//���� 1 			
	#elif defined(TFT35)
	Drow_Touch_Point(20,20);//���� 1 	
	#endif
	while(1) 
	{ 
		
	//	if(Pen_Point.Key_Sta==Key_Down)//���������� 
		{ 
			if(GUI_TOUCH_X_MeasureXY(&x,  &y) == 0)
			{
				continue;
			}

			 if ((x <= 50) ||  (y <= 50) ||(x >= 4060)  ||(y == 4060) )
			    {
			    	continue;
			    }

			/* �ӳ�ȥ�� */
		    /*	tmpTime = touch_time;
			touch_time = getTick();
			diffTime = getTickDiff(tmpTime, touch_time);

			if(diffTime < 10)
			{
				continue;
			}*/
		    /* Convert values into logical values */
			#if GUI_TOUCH_SWAP_XY   /* Is X/Y swapped ? */		   
				tmpCood = x;
				x = y;
				y = tmpCood;
			#endif
			
				//y = GUI_TOUCH_AD_BOTTOM - y;
				pos_temp[cnt][0]=x; 
				pos_temp[cnt][1]=y; 
		//	printf("press:%d,%d\n\r",x, y);
				cnt++; 
			 
			switch(cnt) 
			{ 
				case 1: 
					for(i = 0; i < 65535 * 100; i++);
					GUI_Clear();//���� 
					#if defined(TFT70)
					Drow_Touch_Point(750, 50);//����2 				
					#elif defined(TFT35)
					Drow_Touch_Point(460, 20);//����2 		
					#endif
				break; 

				case 2: 
					for(i = 0; i < 65535 * 100; i++);
					GUI_Clear();//���� 
					#if defined(TFT70)
					Drow_Touch_Point(50, 430);//����3 
					#elif defined(TFT35)
					Drow_Touch_Point(20, 300);//����3 
					#endif
				break; 

				case 3: 
					for(i = 0; i < 65535 * 100; i++);
					GUI_Clear();//���� 
					#if defined(TFT70)
					Drow_Touch_Point(750, 430);//����4 
					#elif defined(TFT35)
					Drow_Touch_Point(460, 300);//����4 
					#endif
				break; 

				case 4: 
					for(i = 0; i < 65535 * 100; i++);
					GUI_Clear();//���� 
					#if defined(TFT70)
					Drow_Touch_Point(400, 240);//����5
					#elif defined(TFT35)
					Drow_Touch_Point(240, 160);//����5
					#endif
				break; 

				case 5:
					//�Ա���� 
					tem1=abs(pos_temp[0][0]-pos_temp[1][0]);//x1-x2 
					/**
						sean 19.10.28
					*/
					if(tem1 <= 2)
					{ 
						cnt=0; 
						GUI_Clear();//���� 
						Drow_Touch_Point(20,20); 
						continue; 
					} 
					
					tem2=abs(pos_temp[0][1]-pos_temp[1][1]);//y1-y2 
					tem1*=tem1; 
					tem2*=tem2; 
					d1=(tem1+tem2);//�õ�1,2�ľ��� 
					tem1=abs(pos_temp[2][0]-pos_temp[3][0]);//x3-x4 
					/**
						sean 19.10.28
					*/
					if(tem1 <= 2)
					{ 
						cnt=0; 
						GUI_Clear();//���� 
						Drow_Touch_Point(20,20); 
						continue; 
					} 
					tem2=abs(pos_temp[2][1]-pos_temp[3][1]);//y3-y4 
					tem1*=tem1; 
					tem2*=tem2; 
					d2=(tem1+tem2);//�õ�3,4�ľ���
					
					fac=(float)((float)d1/(float)d2); 
				//	printf("fac1:%f\n" , fac);
					
					//fac = sqrt(fac);
				
					if(fac<0.6||fac>1.5||d1==0||d2==0)//���ϸ� 
					{ 
						cnt=0; 
						GUI_Clear();//���� 
						#if defined(TFT70)
						Drow_Touch_Point(50,50);//���� 1 			
						#elif defined(TFT35)
						Drow_Touch_Point(20,20);//���� 1 	
						#endif
						continue; 
					} 
					tem1=abs(pos_temp[0][0]-pos_temp[2][0]);//x1-x3 
					tem2=abs(pos_temp[0][1]-pos_temp[2][1]);//y1-y3 
					/**
						sean 19.10.28
					*/
					if(tem2 <= 2)
					{ 
						cnt=0; 
						GUI_Clear();//���� 
						Drow_Touch_Point(20,20); 
						continue; 
					} 
					tem1*=tem1; 
					tem2*=tem2; 
					d1=(tem1+tem2);//�õ�1,3�ľ��� 
					tem1=abs(pos_temp[1][0]-pos_temp[3][0]);//x2-x4 
					tem2=abs(pos_temp[1][1]-pos_temp[3][1]);//y2-y4 
					/**
						sean 19.10.28
					*/
					if(tem2 <= 2)
					{ 
						cnt=0; 
						GUI_Clear();//���� 
						Drow_Touch_Point(20,20); 
						continue; 
					} 
					tem1*=tem1; 
					tem2*=tem2; 
					d2=(tem1+tem2);//�õ�2,4�ľ��� 
					fac=(float)((float)d1/(float)d2); 
				//	printf("fac2:%f\n" , fac);
					
					if(fac<0.6||fac>1.5||d1==0||d2==0)//���ϸ� 
					{ 
						cnt=0; 
						GUI_Clear();//���� 
						#if defined(TFT70)
						Drow_Touch_Point(50,50);//���� 1 		
						#elif defined(TFT35)
						Drow_Touch_Point(20,20);//���� 1 	
						#endif
						continue; 
					}//��ȷ�� 
					//�Խ������ 
					tem1=abs(pos_temp[1][0]-pos_temp[2][0]);//x1-x3 
					tem2=abs(pos_temp[1][1]-pos_temp[2][1]);//y1-y3 
					tem1*=tem1; 
					tem2*=tem2; 
					d1=(tem1+tem2);//�õ�1,4�ľ��� 
					
					tem1=abs(pos_temp[0][0]-pos_temp[3][0]);//x2-x4 
					tem2=abs(pos_temp[0][1]-pos_temp[3][1]);//y2-y4 
					tem1*=tem1; 
					tem2*=tem2; 
					d2=(tem1+tem2);//�õ�2,3�ľ��� 
					fac=(float)((float)d1/(float)d2); 

				//	printf("fac3:%f\n" , fac);
					
					if(fac<0.6||fac>1.5||d1==0||d2==0)//���ϸ� 
					{ 
						cnt=0; 
						GUI_Clear();//���� 
						#if defined(TFT70)
						Drow_Touch_Point(50,50);//���� 1 			
						#elif defined(TFT35)
						Drow_Touch_Point(20,20);//���� 1 	
						#endif 
						continue; 
					}

					//�е�λ���ж�					
					tem1=abs(pos_temp[4][0]-pos_temp[0][0]);//x5-x1
					tem2=abs(pos_temp[4][0]-pos_temp[3][0]);//x5-x4 

					fac=(float)((float)tem1/(float)tem2); 
					
					if(fac<0.6||fac>1.5)//���ϸ� 
					{ 
						cnt=0; 
						GUI_Clear();//���� 
						#if defined(TFT70)
						Drow_Touch_Point(50,50);//���� 1 			
						#elif defined(TFT35)
						Drow_Touch_Point(20,20);//���� 1 	
						#endif
						continue; 
					}

					tem1=abs(pos_temp[4][1]-pos_temp[0][1]);//y5-y1
					tem2=abs(pos_temp[4][1]-pos_temp[3][1]);//y5-y4 

					fac=(float)((float)tem1/(float)tem2); 
					
					if(fac<0.6||fac>1.5)//���ϸ� 
					{ 
						cnt=0; 
						GUI_Clear();//���� 
						#if defined(TFT70)
						Drow_Touch_Point(50,50);//���� 1 			
						#elif defined(TFT35)
						Drow_Touch_Point(20,20);//���� 1 	
						#endif 
						continue; 
					}

					tem1=abs(pos_temp[4][0]-pos_temp[1][0]);//x5-x2
					tem2=abs(pos_temp[4][0]-pos_temp[2][0]);//x5-x3

					fac=(float)((float)tem1/(float)tem2); 
					
					if(fac<0.6||fac>1.5)//���ϸ� 
					{ 
						cnt=0; 
						GUI_Clear();//���� 
						#if defined(TFT70)
						Drow_Touch_Point(50,50);//���� 1 			
						#elif defined(TFT35)
						Drow_Touch_Point(20,20);//���� 1 	
						#endif 
						continue; 
					}

					tem1=abs(pos_temp[4][1]-pos_temp[1][1]);//y5-y2
					tem2=abs(pos_temp[4][1]-pos_temp[2][1]);//y5-y3

					fac=(float)((float)tem1/(float)tem2); 
					
					if(fac<0.6||fac>1.5)//���ϸ� 
					{ 
						cnt=0; 
						GUI_Clear();//���� 
						#if defined(TFT70)
						Drow_Touch_Point(50,50);//���� 1 			
						#elif defined(TFT35)
						Drow_Touch_Point(20,20);//���� 1 	
						#endif 
						continue; 
					}

					#if defined(TFT70)
					xMin_temp[0] = pos_temp[0][0] - ( pos_temp[1][0]-pos_temp[0][0]) * (0 + 50) / 700;
  					xMax_temp[0] = pos_temp[0][0] + ( pos_temp[1][0]-pos_temp[0][0]) * (LCD_XSIZE - 50) / 700;
  					yMin_temp[0] = pos_temp[0][1] - ( pos_temp[2][1]-pos_temp[0][1]) * (0 + 50) / 380;
  					yMax_temp[0] = pos_temp[0][1] + ( pos_temp[2][1]-pos_temp[0][1]) * (LCD_YSIZE - 50) / 380;

					xMin_temp[1] = pos_temp[2][0] - ( pos_temp[3][0]-pos_temp[2][0]) * (0 + 50) / 700;
  					xMax_temp[1] = pos_temp[2][0] + ( pos_temp[3][0]-pos_temp[2][0]) * (LCD_XSIZE - 50) / 700;
  					yMin_temp[1] = pos_temp[1][1] - ( pos_temp[3][1]-pos_temp[1][1]) * (0 + 50) / 380;
  					yMax_temp[1] = pos_temp[1][1] + ( pos_temp[3][1]-pos_temp[1][1]) * (LCD_YSIZE - 50) / 380;
					#if 0
					xMin_temp[2] = pos_temp[4][0] - ( pos_temp[4][0]-pos_temp[0][0]) * (0 + 400) / 350;
					xMax_temp[2] = pos_temp[4][0] + ( pos_temp[3][0]-pos_temp[4][0]) * (LCD_XSIZE - 400) / 350;
					yMin_temp[2] = pos_temp[4][1] - ( pos_temp[4][1]-pos_temp[0][1]) * (0 + 240) / (240-50);
					yMax_temp[2] = pos_temp[4][1] + ( pos_temp[3][1]-pos_temp[4][1]) * (LCD_YSIZE - 240) / (430-240);

					xMin_temp[3] = pos_temp[4][0] - ( pos_temp[4][0]-pos_temp[2][0]) * (0 + 400) / 350;
					xMax_temp[3] = pos_temp[4][0] + ( pos_temp[1][0]-pos_temp[4][0]) * (LCD_XSIZE - 400) / 350;
					yMin_temp[3] = pos_temp[4][1] - ( pos_temp[4][1]-pos_temp[0][1]) * (0 + 240) / (240-50);
					yMax_temp[3] = pos_temp[4][1] + (pos_temp[2][1]-pos_temp[4][1]) * (LCD_YSIZE - 240) / (430-240);

					xMin=(xMin_temp[0]+xMin_temp[1]+xMin_temp[2]+xMin_temp[3])/4;
					xMax=(xMax_temp[0]+xMax_temp[1]+xMax_temp[2]+xMax_temp[3])/4;
					yMin=(yMin_temp[0]+yMin_temp[1]+yMin_temp[2]+yMin_temp[3])/4;
					yMax=(yMax_temp[0]+yMax_temp[1]+yMax_temp[2]+yMax_temp[3])/4;
					#endif
					xMin=(xMin_temp[0]+xMin_temp[1])>>1;
					xMax=(xMax_temp[0]+xMax_temp[1])>>1;
					yMin=(yMin_temp[0]+yMin_temp[1])>>1;
					yMax=(yMax_temp[0]+yMax_temp[1])>>1;
					#elif defined(TFT35)
					xMin = pos_temp[0][0] - ( pos_temp[1][0]-pos_temp[0][0]) * (0 + 20) / 440;
  					xMax = pos_temp[0][0] + ( pos_temp[1][0]-pos_temp[0][0]) * (LCD_XSIZE - 20) / 440;
  					yMin = pos_temp[0][1] - ( pos_temp[2][1]-pos_temp[0][1]) * (0 + 20) / 280;
  					yMax = pos_temp[0][1] + ( pos_temp[2][1]-pos_temp[0][1]) * (LCD_YSIZE - 20) / 280;					
					#endif

					setTouchBound(xMin, xMax, yMax, yMin);

					gCfgItems.touch_adj_xMin = xMin;
					gCfgItems.touch_adj_xMax = xMax;
					gCfgItems.touch_adj_yMin = yMin;
					gCfgItems.touch_adj_yMax = yMax;

					bakup_cfg_inf(BAK_TOUCH_ADJ_XMIN_ID,  gCfgItems.touch_adj_xMin);
					bakup_cfg_inf(BAK_TOUCH_ADJ_XMAX_ID,  gCfgItems.touch_adj_xMax);
					bakup_cfg_inf(BAK_TOUCH_ADJ_YMIN_ID,  gCfgItems.touch_adj_yMin);
					bakup_cfg_inf(BAK_TOUCH_ADJ_YMAX_ID,  gCfgItems.touch_adj_yMax);
					bakup_cfg_inf(BAK_TOUCH_ADJ_FLAG_ID,  0xaa);
								
					GUI_Clear();//���� 
					
					if(gCfgItems.language == LANG_COMPLEX_CHINESE || gCfgItems.language == LANG_SIMPLE_CHINESE)
					{
						GUI_DispStringAt("У�����!", 100, 100);//У����� 
					}
					else
					{
						GUI_DispStringAt("Touch Screen Adjust OK!", 60, 100);//У����� 
					}
					for(i = 0; i < 65535 * 100; i++);

					GUI_SetBkColor(gCfgItems.background_color);
										
					GUI_Clear();//���� 

					GUI_SetFont(&FONT_TITLE);
					GUI_UC_SetEncodeUTF8();

					draw_return_ui();

					return;//У����� 
			} 
		} 
	}
}
#else

void Touch_Adjust()
{
	signed short  pos_temp[5][2];//���껺��ֵ 
	signed short xMin, xMax, yMin, yMax;
	uint8_t cnt=0;  
	uint32_t d1,d2; 
	uint32_t tem1,tem2; 
	float fac; 
	uint16_t x, y, tmpCood;
	float xfac, yfac, xoff, yoff;
	int i;
	int8_t buf[30];
	int32_t tmpTime, diffTime;
	
	
	cnt=0; 

	//SZ_STM32_LCDInit();
	//SZ_TS_Init();
	
	GUI_SetBkColor(GUI_BLUE); 
	GUI_SetColor(GUI_WHITE);
	GUI_SetFont(&GUI_FontHZ_fontHz14);
	GUI_Clear();
	

	
	/*LCD_SetBackColor(LCD_COLOR_WHITE);
	LCD_SetTextColor(LCD_COLOR_BLUE);
	LCD_Clear(LCD_COLOR_WHITE);*/
 
	Drow_Touch_Point(20,20);//���� 1 
	while(1) 
	{ 
		
	//	if(Pen_Point.Key_Sta==Key_Down)//���������� 
		{ 
			if(GUI_TOUCH_X_MeasureXY(&x,  &y) == 0)
			{
				continue;
			}

			 if ((x <= 50) ||  (y <= 50) ||(x >= 4060)  ||(y == 4060) )
			    {
			    	continue;
			    }

			/* �ӳ�ȥ�� */
		    /*	tmpTime = touch_time;
			touch_time = getTick();
			diffTime = getTickDiff(tmpTime, touch_time);

			if(diffTime < 10)
			{
				continue;
			}*/
		    /* Convert values into logical values */
			#if GUI_TOUCH_SWAP_XY   /* Is X/Y swapped ? */		   
				tmpCood = x;
				x = y;
				y = tmpCood;
			#endif
			
				//y = GUI_TOUCH_AD_BOTTOM - y;
				pos_temp[cnt][0]=x; 
				pos_temp[cnt][1]=y; 
		//	printf("press:%d,%d\n\r",x, y);
				cnt++; 
			 
			switch(cnt) 
			{ 
				case 1: 
					GUI_Clear();//���� 
					Drow_Touch_Point(300, 20);//����2 				
				break; 

				case 2: 
					GUI_Clear();//���� 
					Drow_Touch_Point(20, 220);//����3 
				break; 

				case 3: 
					GUI_Clear();//���� 
					Drow_Touch_Point(300, 220);//����4 
				break; 

				case 4: 
					GUI_Clear();//���� 
					Drow_Touch_Point(160, 120);//����5
				break; 

				case 5:
					//�Ա���� 
					tem1=abs(pos_temp[0][0]-pos_temp[1][0]);//x1-x2 
					tem2=abs(pos_temp[0][1]-pos_temp[1][1]);//y1-y2 
					tem1*=tem1; 
					tem2*=tem2; 
					d1=(tem1+tem2);//�õ�1,2�ľ��� 
					tem1=abs(pos_temp[2][0]-pos_temp[3][0]);//x3-x4 
					tem2=abs(pos_temp[2][1]-pos_temp[3][1]);//y3-y4 
					tem1*=tem1; 
					tem2*=tem2; 
					d2=(tem1+tem2);//�õ�3,4�ľ���
					
					fac=(float)((float)d1/(float)d2); 
				//	printf("fac1:%f\n" , fac);
					
					//fac = sqrt(fac);
				
					if(fac<0.6||fac>1.5||d1==0||d2==0)//���ϸ� 
					{ 
						cnt=0; 
						GUI_Clear();//���� 
						Drow_Touch_Point(20,20); 
						continue; 
					} 
					tem1=abs(pos_temp[0][0]-pos_temp[2][0]);//x1-x3 
					tem2=abs(pos_temp[0][1]-pos_temp[2][1]);//y1-y3 
					tem1*=tem1; 
					tem2*=tem2; 
					d1=(tem1+tem2);//�õ�1,3�ľ��� 
					tem1=abs(pos_temp[1][0]-pos_temp[3][0]);//x2-x4 
					tem2=abs(pos_temp[1][1]-pos_temp[3][1]);//y2-y4 
					tem1*=tem1; 
					tem2*=tem2; 
					d2=(tem1+tem2);//�õ�2,4�ľ��� 
					fac=(float)((float)d1/(float)d2); 
				//	printf("fac2:%f\n" , fac);
					
					if(fac<0.6||fac>1.5||d1==0||d2==0)//���ϸ� 
					{ 
						cnt=0; 
						GUI_Clear();//���� 
						Drow_Touch_Point(20,20); 
						continue; 
					}//��ȷ�� 
					//�Խ������ 
					tem1=abs(pos_temp[1][0]-pos_temp[2][0]);//x1-x3 
					tem2=abs(pos_temp[1][1]-pos_temp[2][1]);//y1-y3 
					tem1*=tem1; 
					tem2*=tem2; 
					d1=(tem1+tem2);//�õ�1,4�ľ��� 
					
					tem1=abs(pos_temp[0][0]-pos_temp[3][0]);//x2-x4 
					tem2=abs(pos_temp[0][1]-pos_temp[3][1]);//y2-y4 
					tem1*=tem1; 
					tem2*=tem2; 
					d2=(tem1+tem2);//�õ�2,3�ľ��� 
					fac=(float)((float)d1/(float)d2); 

				//	printf("fac3:%f\n" , fac);
					
					if(fac<0.6||fac>1.5||d1==0||d2==0)//���ϸ� 
					{ 
						cnt=0; 
						GUI_Clear();//���� 
						Drow_Touch_Point(20,20); 
						continue; 
					}

					//�е�λ���ж�					
					tem1=abs(pos_temp[4][0]-pos_temp[0][0]);//x5-x1
					tem2=abs(pos_temp[4][0]-pos_temp[3][0]);//x5-x4 

					fac=(float)((float)tem1/(float)tem2); 
					
					if(fac<0.6||fac>1.5)//���ϸ� 
					{ 
						cnt=0; 
						GUI_Clear();//���� 
						Drow_Touch_Point(20,20); 
						continue; 
					}

					tem1=abs(pos_temp[4][1]-pos_temp[0][1]);//y5-y1
					tem2=abs(pos_temp[4][1]-pos_temp[3][1]);//y5-y4 

					fac=(float)((float)tem1/(float)tem2); 
					
					if(fac<0.6||fac>1.5)//���ϸ� 
					{ 
						cnt=0; 
						GUI_Clear();//���� 
						Drow_Touch_Point(20,20); 
						continue; 
					}

					tem1=abs(pos_temp[4][0]-pos_temp[1][0]);//x5-x2
					tem2=abs(pos_temp[4][0]-pos_temp[2][0]);//x5-x3

					fac=(float)((float)tem1/(float)tem2); 
					
					if(fac<0.6||fac>1.5)//���ϸ� 
					{ 
						cnt=0; 
						GUI_Clear();//���� 
						Drow_Touch_Point(20,20); 
						continue; 
					}

					tem1=abs(pos_temp[4][1]-pos_temp[1][1]);//y5-y2
					tem2=abs(pos_temp[4][1]-pos_temp[2][1]);//y5-y3

					fac=(float)((float)tem1/(float)tem2); 
					
					if(fac<0.6||fac>1.5)//���ϸ� 
					{ 
						cnt=0; 
						GUI_Clear();//���� 
						Drow_Touch_Point(20,20); 
						continue; 
					}

					
					xMin = pos_temp[0][0] - ( pos_temp[1][0]-pos_temp[0][0]) * (0 + 20) / 280;
  					xMax = pos_temp[0][0] + ( pos_temp[1][0]-pos_temp[0][0]) * (LCD_XSIZE - 20) / 280;
  					yMin = pos_temp[0][1] - ( pos_temp[2][1]-pos_temp[0][1]) * (0 + 20) / 200;
  					yMax = pos_temp[0][1] + ( pos_temp[2][1]-pos_temp[0][1]) * (LCD_YSIZE - 20) / 200;

					setTouchBound(xMin, xMax, yMax, yMin);
					
					gCfgItems.touch_adj_xMin = xMin;
					gCfgItems.touch_adj_xMax = xMax;
					gCfgItems.touch_adj_yMin = yMin;
					gCfgItems.touch_adj_yMax = yMax;

					//bakup_cfg_inf(BAK_TOUCH_ADJ_XMIN_ID,  gCfgItems.touch_adj_xMin);
					//bakup_cfg_inf(BAK_TOUCH_ADJ_XMAX_ID,  gCfgItems.touch_adj_xMax);
					//bakup_cfg_inf(BAK_TOUCH_ADJ_YMIN_ID,  gCfgItems.touch_adj_yMin);
					//bakup_cfg_inf(BAK_TOUCH_ADJ_YMAX_ID,  gCfgItems.touch_adj_yMax);
					//bakup_cfg_inf(BAK_TOUCH_ADJ_FLAG_ID,  0xaa);
								
					GUI_Clear();//���� 
					
					if(gCfgItems.language == LANG_ENGLISH)
					{
						GUI_DispStringAt("Touch Screen Adjust OK!", 60, 100);//У����� 
					}
					else
					{
						GUI_DispStringAt("У�����!", 100, 100);
					}
					for(i = 0; i < 65535 * 100; i++);

					GUI_SetBkColor(gCfgItems.background_color);
										
					GUI_Clear();//���� 

					draw_return_ui();

					return;//У����� 
			} 
		} 
	}
}
#endif

void draw_calibrate()
{

	if(disp_state_stack._disp_state[disp_state_stack._disp_index] != CALIBRATE_UI)
	{
		disp_state_stack._disp_index++;
		disp_state_stack._disp_state[disp_state_stack._disp_index] = CALIBRATE_UI;
	}
	disp_state = CALIBRATE_UI;

	Touch_Adjust();
}
