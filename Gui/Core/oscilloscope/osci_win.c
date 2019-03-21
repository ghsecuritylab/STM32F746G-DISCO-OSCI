/**
  ******************************************************************************
  * @file    game_win.c
  * @author  MCD Application Team
  * @brief   game functions
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2016 STMicroelectronics International N.V. 
  * All rights reserved.</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other 
  *    contributors to this software may be used to endorse or promote products 
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this 
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under 
  *    this license is void and will automatically terminate your rights under 
  *    this license. 
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT 
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT 
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "osci_res.c"

#include "oszi.h"

#include "../../../Modules/osci_recorder/osci_if.h"


/** @addtogroup GAME_MODULE
  * @{
  */

/** @defgroup GAME
  * @brief game routines
  * @{
  */
  
/* External variables --------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static void Startup(WM_HWIN hWin, uint16_t xpos, uint16_t ypos);

/* Private typedef -----------------------------------------------------------*/
K_ModuleItem_Typedef  oscilloscope =
{
  7,
  " oscilloscope",
  open_osci,
  0,
  Startup,
  NULL,
}
;

/* Private defines -----------------------------------------------------------*/
#define ID_FRAMEWIN_INFO        (GUI_ID_USER + 0x01)
#define ID_IMAGE_INFO           (GUI_ID_USER + 0x02)
#define ID_PROCESS_TIMER        (GUI_ID_USER + 0x03)

#define ID_BUTTON_EXIT          (GUI_ID_USER + 0x04)
#define ID_BUTTON_TIME          (GUI_ID_USER + 0x05)
#define ID_BUTTON_CHANNEL1      (GUI_ID_USER + 0x06)
#define ID_BUTTON_CHANNEL2      (GUI_ID_USER + 0x07)
#define ID_BUTTON_TRIGGER       (GUI_ID_USER + 0x08)
#define ID_BUTTON_RUN           (GUI_ID_USER + 0x09)
#define ID_BUTTON_MORE          (GUI_ID_USER + 0x0B)

#define ID_BUTTON_BACK          (GUI_ID_USER + 0x0C)
#define ID_BUTTON_CURSOR        (GUI_ID_USER + 0x0D)
#define ID_BUTTON_SAVE          (GUI_ID_USER + 0x0F)
#define ID_BUTTON_HELP          (GUI_ID_USER + 0x12)
#define GUI_ID_LEFT             (GUI_ID_USER + 0x20)
#define GUI_ID_RIGHT            (GUI_ID_USER + 0x21)
#define GUI_ID_UP               (GUI_ID_USER + 0x22)
#define GUI_ID_DOWN             (GUI_ID_USER + 0x23)

#define CLIENT_COLOR      GUI_TRANSPARENT
#define GRID_COLOR        GUI_BROWN

#define RECORDER_MODE_REC_DISABLED            0x00
#define RECORDER_MODE_REC_IDLE                0x01
#define RECORDER_MODE_RECORDING               0x02
#define RECORDER_MODE_PLAY_IDLE               0x03
#define RECORDER_MODE_PLAYING                 0x04


/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static WM_HWIN   _hFrame;
static WM_HWIN   hButton[8];
static WM_HTIMER hProcessTimer = 0;
static WM_HWIN   hCurrentDialog = 0;

/* Time Dialog handling ------------------------------------------------------*/
static WM_HWIN   _TimeDialog = 0;
WM_HWIN CreateTimeDialog(WM_HWIN hParent);
void TimeDialog_SetSelection(TIME_SETTING_e timeSel);

/* Channel Dialog handling ---------------------------------------------------*/
static WM_HWIN   _ChannelDialog = 0;
WM_HWIN CreateChannelDialog(WM_HWIN hParent);
void ChannelDialog_SetConfig(CHANNEL_CONFIG_t* config);


static WM_HWIN   _TriggerDialog = 0;
WM_HWIN CreateTriggerDialog(WM_HWIN hParent);
void TriggerDialog_SetConfig(TRIGGER_CONFIG_t* config);

static WM_HWIN   _CursorDialog = 0;
WM_HWIN CreateCursorDialog(WM_HWIN hParent);
void CursorDialog_SetConfig(CURSOR_CONFIG_t* config);


static uint8_t  RecorderMode = RECORDER_MODE_REC_DISABLED;
static char     FileName[256];



/* Invalidate Board */
static void _InvalidateOsci(void) {
  WM_InvalidateWindow(WM_GetClientWindow(_hFrame));
}




/**
  * @brief Handle PID 
  * @param  x: X position     
  * @param  y: Y position
  * @param Pressed: touch status
  * @retval None
  */
static void _HandlePID(int x, int y, int Pressed)
{
   static int _IsInHandlePID;

   if (_IsInHandlePID++ == 0)
   {
      if(Pressed)
	  {
         if( hCurrentDialog != 0 )
         {
            GUI_RECT rect;
            WM_GetWindowRectEx(hCurrentDialog, &rect);
            if( (x < rect.x0) || (x > rect.x1) ||
                (y < rect.y0) || (y > rect.y1)	)
            {
               // Touch outside window
               WM_HideWindow(hCurrentDialog);
               hCurrentDialog = 0;
            }
         }
      }
   }
   _IsInHandlePID--;
}

/**
  * @brief _OnTouch 
  * @param pMsg : pointer to data structure
  * @retval None
  */
static void _OnTouch(WM_MESSAGE* pMsg)
{
  const GUI_PID_STATE* pState = (const GUI_PID_STATE*)pMsg->Data.p;
  if (pState) 
  {
    _HandlePID(pState->x, pState->y, pState->Pressed);
  }
}

/**
  * @brief  Paints callback 
  * @param  hWin: pointer to the parent handle
  * @retval None
  */
static void _OnPaint(WM_HWIN hWin)
{
  GUI_COLOR Color;
  GUI_RECT  r;
  int x, y, xPos, yPos;
  int CellSize, rStone, rMove;
  char Cell, IsValidMove;
  int xCircle, yCircle;

  #if AA_USE_HIRES
    GUI_AA_EnableHiRes();
  #endif

  WM_GetClientRectEx(hWin, &r);


  oszi_process();


}




WM_HWIN Id_Active = 0;
int menuPage = 0;

/**
  * @brief  Paints exit button
  * @param  hObj: button handle
  * @retval None
  */
static void _OnPaint_BUTTONS(WM_HWIN hBtn)
{
   GUI_SetBkColor(FRAMEWIN_GetDefaultClientColor());
   GUI_Clear();

   if(menuPage == 0)
      {
         GUI_SetColor(GUI_LIGHTGRAY);
         GUI_SetBkColor(GUI_LIGHTGRAY);
      }
      else if( hBtn == hButton[5] ) // [RUN]
      {
         if( RecorderMode == RECORDER_MODE_RECORDING )
         {
            GUI_SetColor(GUI_RED);
            GUI_SetBkColor(GUI_RED);
         }
         else
         {
            GUI_SetColor(GUI_STCOLOR_LIGHTBLUE);
            GUI_SetBkColor(GUI_STCOLOR_LIGHTBLUE);
         }
      }
      else if( hBtn == Id_Active )
      {
         GUI_SetColor(GUI_LIGHTRED);
         GUI_SetBkColor(GUI_LIGHTRED);
      }
      else
      {
         GUI_SetColor(GUI_STCOLOR_LIGHTBLUE);
         GUI_SetBkColor(GUI_STCOLOR_LIGHTBLUE);
	  if( hBtn == hButton[0] ) // [Back]


  GUI_SetColor(GUI_BLACK);
  GUI_SetFont(GUI_FONT_13B_ASCII);
  const char * str;
  if(menuPage == 0)
  {
    if( hBtn == hButton[0] )
      str = "Exit";
    else if( hBtn == hButton[1] )
      str = "Time";
    else if( hBtn == hButton[2] )
      str = "CH1";
    else if( hBtn == hButton[3] )
      str = "CH2";
    else if( hBtn == hButton[4] )
      str = "Trig";
    else if( hBtn == hButton[5] )
    {
      if( Menu.trigger.state==TRIGGER_STATE_STOP )
        str = "HOLD";
      else
   	    str = "RUN";
    }
    else if( hBtn == hButton[6] )
    {
      if( RecorderMode == RECORDER_MODE_RECORDING )
        str = "REC";
      else
   	    str = "Rec";
    }
    else if( hBtn == hButton[7] )
      str = "More";
    else
      str = "";
  }
  else
  {
    if( hBtn == hButton[0] )
      str = "Back";
    else if( hBtn == hButton[1] )
      str = "Curs";
    else if( hBtn == hButton[2] )
      str = "FFT";
    else if( hBtn == hButton[3] )
      str = "Save";
    else if( hBtn == hButton[4] )
      str = "Send";
    else if( hBtn == hButton[5] )
      str = "Vers";
    else if( hBtn == hButton[6] )
      str = "";
  }
  GUI_DispStringAt(str, 7, 10);
}

void ExecTimeSelectDialog(void);
void ExecChannelConfigDialog(void);
void ExecArrowsWindow(void);

/**
  * @brief  List up to 25 file on the root directory with extension .BMP
  * @param  DirName: Directory name
  * @param  Files: Buffer to contain read files
  * @retval The number of the found files
  */
static void _OsciBuildFileName(void)
{
   static const char* DirName = "0:/Osci";
   int32_t maxFileNum = -1;
   uint32_t counter = 0;
   FRESULT res;
   DIR MyDirectory;
   FILINFO MyFileInfo;
   char newFileName[13] = "00000000.wav\0";

   res = f_opendir(&MyDirectory, DirName);

   if(res == FR_OK)
   {
      for (;;)
      {
         res = f_readdir(&MyDirectory, &MyFileInfo);
         if(res != FR_OK || MyFileInfo.fname[0] == 0)
            break;
         if(MyFileInfo.fname[0] == '.')
            continue;

         if(!(MyFileInfo.fattrib & AM_DIR))
         {
        	 // Searching the "." in the name
            do
            {
               counter++;
            }
            while (MyFileInfo.fname[counter] != 0x2E);


            if((MyFileInfo.fname[counter + 1] == 'w') && (MyFileInfo.fname[counter + 2] == 'a') && (MyFileInfo.fname[counter + 3] == 'v'))
            {
            	// Found a wave file (*.wav)
            	int32_t currentFileNum = atol(MyFileInfo.fname);
            	if( maxFileNum < currentFileNum )
            	{
            		maxFileNum = currentFileNum;
            	}
            }
            counter = 0;
         }
      }
      f_closedir(&MyDirectory);
      maxFileNum++;
#ifdef USE_SD_CARD_DISK
      sprintf(FileName, "0:/Osci/%08ld.wav", maxFileNum);
#else
      sprintf(FileName, "0:/Osci/%08ld.wav", maxFileNum);
#endif
   }
}

void osciStartStopRecording(void)
{
   if( RecorderMode == RECORDER_MODE_REC_IDLE) /* Start Recored */
   {
      _OsciBuildFileName();

      //hItem = WM_GetDialogItem(hMainWin, ID_FILENAMECAPTION);
      //TEXT_SetText(hItem, "File : ");
      //WM_InvalidateWindow(hItem);
      //WM_Update(hItem);
      //hItem = WM_GetDialogItem(hMainWin, ID_FILENAME);
      //TEXT_SetText(hItem, FileName);
      //WM_InvalidateWindow(hItem);
      //WM_Update(hItem);

      OSCI_RECORDER_SelectFile(FileName, FA_CREATE_ALWAYS | FA_WRITE);
      OSCI_RECORDER_StartRec(osciGetSampleFrequency());
      RecorderMode = RECORDER_MODE_RECORDING;

      //hItem = WM_GetDialogItem(pMsg->hWin, ID_EQUAL);
      //IMAGE_SetGIF(hItem, equal, sizeof(equal));
      //WM_InvalidateWindow(hItem);
      //WM_Update(hItem);
   }

   else if( RecorderMode == RECORDER_MODE_RECORDING) /* Cancel */
   {
	  OSCI_RECORDER_StopRec();
      RecorderMode = RECORDER_MODE_REC_IDLE;
//      OSCI_RECORDER_RemoveOsciFile(FileName);
   }
#if 0
   else if( RecorderMode == RECORDER_MODE_PLAY_IDLE) /* Start Play */
   {
      RecorderMode = RECORDER_MODE_PLAYING;
      OSCI_RECORDER_SelectFile(FileName, FA_OPEN_EXISTING | FA_READ);
      OSCI_RECORDER_Play(DEFAULT_AUDIO_IN_FREQ);

      //hItem = WM_GetDialogItem(pMsg->hWin, ID_EQUAL);
      //IMAGE_SetGIF(hItem, equal, sizeof(equal));
      //WM_InvalidateWindow(hItem);
      //WM_Update(hItem);
   }
#endif
   //hItem = WM_GetDialogItem(pMsg->hWin, ID_RECORD_REC_CANCEL_PLAY);
   //WM_InvalidateWindow(hItem);
   //WM_Update(hItem);

   //hItem = WM_GetDialogItem(pMsg->hWin, ID_RECORD_STOP);
   //WM_InvalidateWindow(hItem);
   //WM_Update(hItem);

   //hItem = WM_GetDialogItem(pMsg->hWin, ID_RECORD_PAUSE);
   //WM_InvalidateWindow(hItem);
   //WM_Update(hItem);
}

void handleMenuItemTouch(WM_HWIN hItem)
{
   // Reset the previous selected Item and hide a previous shown dialog
   {
	  else if( hItem == hButton[1] ) // Time
	  {
		 GUI.akt_menu=MM_TIME;
		 {
		 	   int ts = (int)(Menu.timebase.value);
			   TimeDialog_SetSelection((TIME_SETTING_e)ts+1);
			   hCurrentDialog = _TimeDialog;
			   WM_ShowWindow(_TimeDialog);
		 }
	  }
	  else if( hItem == hButton[2] ) // CH1
	  {
	 	 GUI.akt_menu=MM_CH_VIS;
			   chConf.channel  = CHANNEL_SEL_CH1;
			   chConf.voltage  = Menu.ch1.faktor+1;
			   chConf.visible  = !Menu.ch1.visible;
			   chConf.position = Menu.ch1.position;
			   ChannelDialog_SetConfig(&chConf);
			   hCurrentDialog = _ChannelDialog;
			   WM_ShowWindow(_ChannelDialog);
		 }
	  }
	  else if( hItem == hButton[3] ) // CH2
	  {
		 GUI.akt_menu=MM_CH_VIS;
		 {
		    if( WM_IsVisible(_ChannelDialog) )
	  }
	  else if( hItem == hButton[4] ) // TRIG
	  {
		 GUI.akt_menu=MM_TRG_SOURCE;
		 {
		    if( WM_IsVisible(_TriggerDialog) )
	  }
	  else if( hItem == hButton[7] ) // More
   {
	  if( hItem == hButton[0] ) // Back
	  {
	  else if( hItem == hButton[1] ) // CURSOR
	  {
		 GUI.akt_menu=MM_CUR_MODE;
		 {
		    if( WM_IsVisible(_CursorDialog) )
	  }
	  else if( hItem == hButton[1] ) // FFT

int getActiveMenuItem(void)
{
   if(menuPage == 0)
   {
      if( Id_Active == hButton[1] ) // Time
      {
	      return 1;
      }
      else if( Id_Active == hButton[2] ) // CH1
      {
	      return 2;
      }
      else if( Id_Active == hButton[3] ) // CH2
      {
	      return 3;
      }
      else if( Id_Active == hButton[4] ) // Trig
      {
	      return 4;
      }
      else if( Id_Active == hButton[5] ) // Hold
      {
	      return 5;
      }
      else if( Id_Active == hButton[6] ) // Rec
      {
	      return 6;
      }
    }
    else
    {
      if( Id_Active == hButton[1] ) // Cursor
      {
	      return 11;
      }
      else if( Id_Active == hButton[2] ) // FFT
      {
	      return 12;
      }
      else if( Id_Active == hButton[3] ) // Save
      {
	      return 13;
      }
      else if( Id_Active == hButton[4] ) // Send
      {
	      return 14;
      }
      else if( Id_Active == hButton[5] ) // Vers
      {
	      return 15;
      }
      else if( Id_Active == hButton[6] ) // Help
   return 0;
}


/**
  * @brief  callback for all menu buttons
  * @param  pMsg: pointer to data structure of type WM_MESSAGE
  * @retval None
  */
static void _cbButton_menu(WM_MESSAGE * pMsg)
{
  const GUI_PID_STATE* pState = 0;

  switch (pMsg->MsgId)
  {
    case WM_PAINT:
      _OnPaint_BUTTONS(pMsg->hWin);
      break;
    case WM_TOUCH:
   	  pState = (const GUI_PID_STATE*)pMsg->Data.p;
   	  if (pState && pState->Pressed)
   	  {
         handleMenuItemTouch(pMsg->hWin);
   	  }
      BUTTON_Callback(pMsg); // WW: Continue the notification chain...
	  break;
    default:
      /* The original callback */
      BUTTON_Callback(pMsg);
      break;
  }
}



/**
  * @brief  callback Reversi Win 
  * @param pMsg: pointer to data structure 
  * @retval None
  */
static void _cbOsciWin(WM_MESSAGE* pMsg)
{
  WM_HWIN hWin = pMsg->hWin;
  int Id, NCode;
  
  switch (pMsg->MsgId)
  {
#if 0
  case WM_CREATE:
    hProcessTimer = WM_CreateTimer(pMsg->hWin, ID_PROCESS_TIMER, 100, 0);
    break;
#endif
  case WM_TIMER:
	_InvalidateOsci();
    WM_RestartTimer(pMsg->Data.v, 50);
    break;

  case WM_PAINT:
    _OnPaint(hWin);
    break;
  case WM_TOUCH:
    _OnTouch(pMsg);
    break;

  case WM_NOTIFY_PARENT:
    Id    = WM_GetId(pMsg->hWinSrc);    /* Id of widget */
    NCode = pMsg->Data.v;               /* Notification code */
    
    switch(Id)
    {
    case ID_BUTTON_EXIT: 
      switch(NCode)
      {
      case WM_NOTIFICATION_RELEASED:
        if(menuPage == 0)
        {
          //GUI_EndDialog(pMsg->hWin, 0);
        }
        break;
      }
      break;
    }
    break;
#endif
  case WM_DELETE:
    if(hProcessTimer!=0)
    {
       WM_DeleteTimer(hProcessTimer);
       hProcessTimer = 0;
    }
    _hFrame = 0;

    if(_TimeDialog!=0)
    {
      GUI_EndDialog(_TimeDialog, 0);
	  _TimeDialog = 0;
    }
    if(_ChannelDialog!=0)
    {
      GUI_EndDialog(_ChannelDialog, 0);
	  _ChannelDialog = 0;
    }
    if(_TriggerDialog!=0)
    {
      GUI_EndDialog(_TriggerDialog, 0);
	  _TriggerDialog = 0;
    }
    if(_CursorDialog!=0)
    {
      GUI_EndDialog(_CursorDialog, 0);
	  _CursorDialog = 0;
    }
    break;

  default:
    WM_DefaultProc(pMsg);
  }
}

/**
  * @brief  Game window Startup
  * @param  hWin: pointer to the parent handle.
  * @param  xpos: X position 
  * @param  ypos: Y position
  * @retval None
  */
static void Startup(WM_HWIN hWin, uint16_t xpos, uint16_t ypos)
{
  WM_HWIN hItem;
  _hFrame = WINDOW_CreateEx(xpos, ypos, 480, 272, hWin, WM_CF_SHOW, 0, 0x500, &_cbOsciWin);

  hButton[0] = BUTTON_CreateAsChild(440,  0, 40, 34, _hFrame, ID_BUTTON_EXIT, WM_CF_SHOW);
  WM_SetCallback(hButton[0], _cbButton_menu);
  hButton[1] = BUTTON_CreateAsChild(440, 34, 40, 34, _hFrame, ID_BUTTON_TIME, WM_CF_SHOW);
  WM_SetCallback(hButton[1], _cbButton_menu);
  hButton[2] = BUTTON_CreateAsChild(440, 68, 40, 34, _hFrame, ID_BUTTON_CHANNEL1, WM_CF_SHOW);
  WM_SetCallback(hButton[2], _cbButton_menu);
  hButton[3] = BUTTON_CreateAsChild(440,102, 40, 34, _hFrame, ID_BUTTON_CHANNEL2, WM_CF_SHOW);
  WM_SetCallback(hButton[3], _cbButton_menu);
  hButton[4] = BUTTON_CreateAsChild(440,136, 40, 34, _hFrame, ID_BUTTON_TRIGGER, WM_CF_SHOW);
  WM_SetCallback(hButton[4], _cbButton_menu);
  hButton[5] = BUTTON_CreateAsChild(440,170, 40, 34, _hFrame, ID_BUTTON_RUN, WM_CF_SHOW);
  WM_SetCallback(hButton[5], _cbButton_menu);
  hButton[6] = BUTTON_CreateAsChild(440,204, 40, 34, _hFrame, ID_BUTTON_RECORD, WM_CF_SHOW);
  WM_SetCallback(hButton[6], _cbButton_menu);
  hButton[7] = BUTTON_CreateAsChild(440,238, 40, 34, _hFrame, ID_BUTTON_MORE, WM_CF_SHOW);
  WM_SetCallback(hButton[7], _cbButton_menu);

  WINDOW_SetBkColor(_hFrame, GUI_BLACK);

  _TimeDialog = CreateTimeDialog(_hFrame);
  TimeDialog_SetSelection(TIME_SET_5ms);

  _ChannelDialog = CreateChannelDialog(_hFrame);


  _TriggerDialog = CreateTriggerDialog(_hFrame);
  _CursorDialog = CreateCursorDialog(_hFrame);

  //TODO:WW: Check if the Recording drive and folder is available
  OSCI_RECORDER_Init(50);
  RecorderMode = RECORDER_MODE_REC_IDLE;

  oszi_init();
  oszi_start();
  hProcessTimer = WM_CreateTimer(_hFrame, ID_PROCESS_TIMER, 50, 0);
}

/**
  * @brief  Notify State Change
  * @param  pMsg: pointer to data structure of type WM_MESSAGE
  * @retval None
  */
void _cbOsciNotifyStateChange (void)
{
  WM_HWIN hItem;
  if(OSCI_RECORDER_GetState() == OSCI_RECORDER_SUSPENDED)
  {
    if(RecorderMode == RECORDER_MODE_PLAYING)
    {
      RecorderMode = RECORDER_MODE_PLAY_IDLE;
      //hItem = WM_GetDialogItem(hMainWin, ID_RECORD_REC_CANCEL_PLAY);
      //WM_InvalidateWindow(hItem);
      //WM_Update(hItem);
    }

    if(RecorderMode == RECORDER_MODE_RECORDING)
    {
      RecorderMode = RECORDER_MODE_REC_IDLE;
    }

    //hItem = WM_GetDialogItem(hMainWin, ID_EQUAL);
    //IMAGE_SetBitmap(hItem, &bmframe0);

    //WM_InvalidateWindow(hItem);
    //WM_Update(hItem);

    //if(hMainWin != 0)
    //{
    //  hItem = WM_GetDialogItem(hMainWin, ID_ELAPSED_TIME);
    //  TEXT_SetText(hItem, "00:00");
    //  WM_Update(hItem);
    //}
  }
}

#if 0 // MessageBox Example
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/