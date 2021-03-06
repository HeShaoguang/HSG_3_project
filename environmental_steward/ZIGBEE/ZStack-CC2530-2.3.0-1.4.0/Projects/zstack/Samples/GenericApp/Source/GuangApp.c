

/*********************************************************************
 * INCLUDES
 */
#include <string.h>
#include <stdio.h>
#include "OSAL.h"
#include "AF.h"
#include "ZDApp.h"
#include "ZDObject.h"
#include "ZDProfile.h"

#include "GuangApp.h"
#include "DebugTrace.h"

#if !defined( WIN32 )
  #include "OnBoard.h"
#endif

/* HAL */
#include "hal_lcd.h"
#include "hal_led.h"
#include "hal_key.h"
#include "hal_uart.h"
#include "MT_UART.h"

#include "DHT11.H"


// This list should be filled with Application specific Cluster IDs.
const cId_t GuangApp_ClusterList[GENERICAPP_MAX_CLUSTERS] =
{
  GENERICAPP_CLUSTERID
};

const SimpleDescriptionFormat_t GuangApp_SimpleDesc =
{
  GENERICAPP_ENDPOINT,              //  int Endpoint;
  GENERICAPP_PROFID,                //  uint16 AppProfId[2];
  GENERICAPP_DEVICEID,              //  uint16 AppDeviceId[2];
  GENERICAPP_DEVICE_VERSION,        //  int   AppDevVer:4;
  GENERICAPP_FLAGS,                 //  int   AppFlags:4;
  GENERICAPP_MAX_CLUSTERS,          //  byte  AppNumInClusters;
  (cId_t *)GuangApp_ClusterList,  //  byte *pAppInClusterList;
  GENERICAPP_MAX_CLUSTERS,          //  byte  AppNumInClusters;
  (cId_t *)GuangApp_ClusterList   //  byte *pAppInClusterList;
};

// This is the Endpoint/Interface description.  It is defined here, but
// filled-in in GuangApp_Init().  Another way to go would be to fill
// in the structure here and make it a "const" (in code space).  The
// way it's defined in this sample app it is define in RAM.
endPointDesc_t GuangApp_epDesc;

/*********************************************************************
 * EXTERNAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */
byte GuangApp_TaskID;   // Task ID for internal task/event processing
                          // This variable will be received when
                          // GuangApp_Init() is called.
devStates_t GuangApp_NwkState;


byte GuangApp_TransID;  // This is the unique message ID (counter)

afAddrType_t GuangApp_DstAddr;   //????
afAddrType_t GuangApp_Broadcast;   //????


#define GUANG_APP_TX_MAX  20    //????????????  ????
static uint8 GuangApp_TxBuf[GUANG_APP_TX_MAX+1];   //????????
static uint8 GuangApp_TxLen;   //????????????
static uint8 Light_Work_State;   //??????????
static uint8 Fans_Work_State;   //????????????
static uint8 Feng_Work_State;   //????????????

#define FANS      P1_5           //????
#define FENG      P1_4            //??????????????
#define MYLED     P1_1            //??????????        
#define MQ_PIN    P0_6            //??????????????????????  
#define LIGHT_PIN P1_6            //?????????????????? 
/*********************************************************************
 * LOCAL FUNCTIONS
 */
void GuangApp_ProcessZDOMsgs( zdoIncomingMsg_t *inMsg );
void GuangApp_HandleKeys( byte shift, byte keys );
void GuangApp_MessageMSGCB( afIncomingMSGPacket_t *pckt );
void GuangApp_SendTheMessage( void );

/*********************************************************************
 * NETWORK LAYER CALLBACKS
 */

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/*********************************************************************
 * @fn      GuangApp_Init
 *
 * @brief   Initialization function for the Generic App Task.
 *          This is called during initialization and should contain
 *          any application specific initialization (ie. hardware
 *          initialization/setup, table initialization, power up
 *          notificaiton ... ).
 *
 * @param   task_id - the ID assigned by OSAL.  This ID should be
 *                    used to send messages and set timers.
 *
 * @return  none
 */
void GuangApp_Init( byte task_id )
{
  GuangApp_TaskID = task_id;
  GuangApp_NwkState = DEV_INIT;
  GuangApp_TransID = 0;

  //------------------------????????---------------------------------
  MT_UartInit();                    //??????????   ??????????????????
  MT_UartRegisterTaskID(task_id);   //????????????
//  HalUARTWrite(0,"\n strTemp\n", sizeof("\n strTemp\n"));//????????
  //-----------------------------------------------------------------

   //----------??????IO??-------------------
  P0SEL &= 0x3f;                //P0_6 7??????????IO??  0011 1111
  P1SEL &= 0x8d;                //P1_1 4 5 6??????????IO??  1000 1101
  P0DIR &= ~0xc0;               //P0_6 7??????????????  0011 1111
  P1DIR |= 0x32;                 //P1_1 4 5??????????????   0011 0010
  P1DIR &= ~0x40;               //P1_6??????????????   

  //----------------------------------------
  Light_Work_State = 0;
  Feng_Work_State = 0;
  Fans_Work_State = 0; //???????? ?????? ?? ????????????????
  MYLED = 0;                 //??????    
  FANS = 0;                 //??????
  FENG = 1;                 //????????
  
    // Broadcast to everyone ????????:????????
  GuangApp_Broadcast.addrMode = (afAddrMode_t)AddrBroadcast;//????
  GuangApp_Broadcast.endPoint = GENERICAPP_ENDPOINT; //??????????
  GuangApp_Broadcast.addr.shortAddr = 0xFFFF;//??????????????????????????
  
  GuangApp_DstAddr.addrMode = (afAddrMode_t)Addr16Bit;
  GuangApp_DstAddr.endPoint = GENERICAPP_ENDPOINT;
  GuangApp_DstAddr.addr.shortAddr = 0x0000;

  // Fill out the endpoint description.
  GuangApp_epDesc.endPoint = GENERICAPP_ENDPOINT;
  GuangApp_epDesc.task_id = &GuangApp_TaskID;
  GuangApp_epDesc.simpleDesc
            = (SimpleDescriptionFormat_t *)&GuangApp_SimpleDesc;
  GuangApp_epDesc.latencyReq = noLatencyReqs;

  // Register the endpoint description with the AF
  afRegister( &GuangApp_epDesc );

  // Register for all key events - This app will handle all key events
  RegisterForKeys( GuangApp_TaskID );

  // Update the display
#if defined ( LCD_SUPPORTED )
    HalLcdWriteString( "GuangApp", HAL_LCD_LINE_1 );
#endif
    
//  ZDO_RegisterForZDOMsg( GuangApp_TaskID, End_Device_Bind_rsp );    ???
//  ZDO_RegisterForZDOMsg( GuangApp_TaskID, Match_Desc_rsp );      ???
}

/*********************************************************************
 * @fn      GuangApp_ProcessEvent
 *
 * @brief   Generic Application Task event processor.  This function
 *          is called to process all events for the task.  Events
 *          include timers, messages and any other user defined events.
 *
 * @param   task_id  - The OSAL assigned task ID.
 * @param   events - events to process.  This is a bit map and can
 *                   contain more than one event.
 *
 * @return  none
 */
UINT16 GuangApp_ProcessEvent( byte task_id, UINT16 events )
{
  afIncomingMSGPacket_t *MSGpkt;
  afDataConfirm_t *afDataConfirm;
  
  // Data Confirmation message fields
  byte sentEP;
  ZStatus_t sentStatus;
  byte sentTransID;       // This should match the value sent
  (void)task_id;  // Intentionally unreferenced parameter

  byte lineone[12];
  
  if ( events & SYS_EVENT_MSG )
  {
    MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive( GuangApp_TaskID );
    while ( MSGpkt )
    {
      switch ( MSGpkt->hdr.event )
      {
        case ZDO_CB_MSG:
          GuangApp_ProcessZDOMsgs( (zdoIncomingMsg_t *)MSGpkt );
          break;
          
        case KEY_CHANGE:
          GuangApp_HandleKeys( ((keyChange_t *)MSGpkt)->state, ((keyChange_t *)MSGpkt)->keys );
          break;

        case AF_DATA_CONFIRM_CMD:
          // This message is received as a confirmation of a data packet sent.
          // The status is of ZStatus_t type [defined in ZComDef.h]
          // The message fields are defined in AF.h
          afDataConfirm = (afDataConfirm_t *)MSGpkt;
          sentEP = afDataConfirm->endpoint;
          sentStatus = afDataConfirm->hdr.status;
          sentTransID = afDataConfirm->transID;
          (void)sentEP;
          (void)sentTransID;

          // Action taken when confirmation is received.
          if ( sentStatus != ZSuccess )
          {
            // The data wasn't delivered -- Do something
          }
          break;

        case AF_INCOMING_MSG_CMD:
          GuangApp_MessageMSGCB( MSGpkt );
          break;

        case ZDO_STATE_CHANGE:
          
          
          
          GuangApp_NwkState = (devStates_t)(MSGpkt->hdr.status);
          if (GuangApp_NwkState == DEV_END_DEVICE)
          {
            lineone[0] = 'E';
            lineone[1] = 'n';
            lineone[2] = 'd';
            lineone[3] = 'D';

            lineone[4] = 'e';
            lineone[5] = 'v';
            lineone[6] = 'i';
            lineone[7] = 'c';
            lineone[8] = 'e';
            lineone[9] = ':';
            lineone[10] = ZIGBEE_ID;
            lineone[11] = '\0';   
            
          }else if(GuangApp_NwkState == DEV_ROUTER)
          {
            
            lineone[0] = 'R';
            lineone[1] = 'o';
            lineone[2] = 'u';
            lineone[3] = 't';

            lineone[4] = 'e';
            lineone[5] = 'r';
            lineone[6] = ':';
            lineone[7] = ZIGBEE_ID;
            lineone[8] = '\0';
            
          }
          
          
          
          if ( //(GuangApp_NwkState == DEV_ZB_COORD)|| 
              (GuangApp_NwkState == DEV_ROUTER)
              ||(GuangApp_NwkState == DEV_END_DEVICE) )
              {
                LcdClearLine(0,8);  //????
                LCD_P8x16Str(0, 0, lineone); //id
                LCD_P16x16Ch(16,2,4*16);   //??
                LCD_P16x16Ch(32,2,5*16);  //??
                LCD_P16x16Ch(96,2,6*16);  //??
                LCD_P16x16Ch(112,2,7*16);  //??
              for(int i=0; i<3; i++) //?????????????????????? 
              { 
                if(i==0) 
                { 
                  LCD_P16x16Ch(i*16,4,i*16); 
                  LCD_P16x16Ch(i*16,6,(i+3)*16); 
                } 
                else 
                { 
                  LCD_P16x16Ch(i*16,4,i*16); 
                  LCD_P16x16Ch(i*16,6,i*16); 
                } 
              } 
              
            // Start sending "the" message in a regular interval.
            osal_start_timerEx( GuangApp_TaskID,
                                GENERICAPP_SEND_MSG_EVT,
                              GENERICAPP_SEND_MSG_TIMEOUT );
          }
          break;

        default:
          break;
      }

      // Release the memory
      osal_msg_deallocate( (uint8 *)MSGpkt );

      // Next
      MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive( GuangApp_TaskID );
    }

    // return unprocessed events
    return (events ^ SYS_EVENT_MSG);
  }

  // Send a message out - This event is generated by a timer
  //  (setup in GuangApp_Init()).
  if ( events & GENERICAPP_SEND_MSG_EVT )
  {
    // Send "the" message
    GuangApp_SendTheMessage();

    // Setup to send message again
    osal_start_timerEx( GuangApp_TaskID,
                        GENERICAPP_SEND_MSG_EVT,
                      GENERICAPP_SEND_MSG_TIMEOUT );

    // return unprocessed events
    return (events ^ GENERICAPP_SEND_MSG_EVT);
  }

  // Discard unknown events
  return 0;
}

/*********************************************************************
 * Event Generation Functions
 */

/*********************************************************************
 * @fn      GuangApp_ProcessZDOMsgs()
 *
 * @brief   Process response messages
 *
 * @param   none
 *
 * @return  none
 */
void GuangApp_ProcessZDOMsgs( zdoIncomingMsg_t *inMsg )
{
  switch ( inMsg->clusterID )
  {
    case End_Device_Bind_rsp:
      if ( ZDO_ParseBindRsp( inMsg ) == ZSuccess )
      {
        // Light LED
        HalLedSet( HAL_LED_4, HAL_LED_MODE_ON );
      }
#if defined(BLINK_LEDS)
      else
      {
        // Flash LED to show failure
        HalLedSet ( HAL_LED_4, HAL_LED_MODE_FLASH );
      }
#endif
      break;

    case Match_Desc_rsp:
      {
        ZDO_ActiveEndpointRsp_t *pRsp = ZDO_ParseEPListRsp( inMsg );
        if ( pRsp )
        {
          if ( pRsp->status == ZSuccess && pRsp->cnt )
          {
            GuangApp_DstAddr.addrMode = (afAddrMode_t)Addr16Bit;
            GuangApp_DstAddr.addr.shortAddr = pRsp->nwkAddr;
            // Take the first endpoint, Can be changed to search through endpoints
            GuangApp_DstAddr.endPoint = pRsp->epList[0];

            // Light LED
            HalLedSet( HAL_LED_4, HAL_LED_MODE_ON );
          }
          osal_mem_free( pRsp );
        }
      }
      break;
  }
}

/*********************************************************************
 * @fn      GuangApp_HandleKeys
 *
 * @brief   Handles all key events for this device.
 *
 * @param   shift - true if in shift/alt.
 * @param   keys - bit field for key events. Valid entries:
 *                 HAL_KEY_SW_4
 *                 HAL_KEY_SW_3
 *                 HAL_KEY_SW_2
 *                 HAL_KEY_SW_1
 *
 * @return  none
 */
void GuangApp_HandleKeys( byte shift, byte keys )
{
 // zAddrType_t dstAddr;
  
  // Shift is used to make each button/switch dual purpose.
  if ( shift )
  {
    if ( keys & HAL_KEY_SW_1 )
    {
    }
    if ( keys & HAL_KEY_SW_2 )
    {
    }
    if ( keys & HAL_KEY_SW_3 )
    {
    }
    if ( keys & HAL_KEY_SW_4 )
    {
    }
  }
  else
  {
    if ( keys & HAL_KEY_SW_1 )
    {
    }

    if ( keys & HAL_KEY_SW_2 )
    {
//      HalLedSet ( HAL_LED_4, HAL_LED_MODE_OFF );
//
//      // Initiate an End Device Bind Request for the mandatory endpoint
//      dstAddr.addrMode = Addr16Bit;
//      dstAddr.addr.shortAddr = 0x0000; // Coordinator
//      ZDP_EndDeviceBindReq( &dstAddr, NLME_GetShortAddr(), 
//                            GuangApp_epDesc.endPoint,
//                            GENERICAPP_PROFID,
//                            GENERICAPP_MAX_CLUSTERS, (cId_t *)GuangApp_ClusterList,
//                            GENERICAPP_MAX_CLUSTERS, (cId_t *)GuangApp_ClusterList,
//                            FALSE );
    }

    if ( keys & HAL_KEY_SW_3 )
    {
    }

    if ( keys & HAL_KEY_SW_4 )
    {
//      HalLedSet ( HAL_LED_4, HAL_LED_MODE_OFF );
//      // Initiate a Match Description Request (Service Discovery)
//      dstAddr.addrMode = AddrBroadcast;
//      dstAddr.addr.shortAddr = NWK_BROADCAST_SHORTADDR;
//      ZDP_MatchDescReq( &dstAddr, NWK_BROADCAST_SHORTADDR,
//                        GENERICAPP_PROFID,
//                        GENERICAPP_MAX_CLUSTERS, (cId_t *)GuangApp_ClusterList,
//                        GENERICAPP_MAX_CLUSTERS, (cId_t *)GuangApp_ClusterList,
//                        FALSE );
    }
  }
}

/*********************************************************************
 * LOCAL FUNCTIONS
 */

/*********************************************************************
 * @fn      GuangApp_MessageMSGCB
 *
 * @brief   Data message processor callback.  This function processes
 *          any incoming data - probably from other devices.  So, based
 *          on cluster ID, perform the intended action.
 *
 * @param   none
 *
 * @return  none
 */
void GuangApp_MessageMSGCB( afIncomingMSGPacket_t *pkt )
{
  switch ( pkt->clusterId )
  {
    case GENERICAPP_CLUSTERID:   //????????????
      
      HalUARTWrite(0, "Rx:", 3); //???????? 
      HalUARTWrite(0, pkt->cmd.Data, pkt->cmd.DataLength); //???????????????? 
      HalUARTWrite(0, "\n", 1); //???????? 

      HalUARTWrite(1, pkt->cmd.Data, pkt->cmd.DataLength); //???????????????? 
 
    break;
      
    case GENERICAPP_BROADCASTID:   //????????????
      
      HalUARTWrite(0, "Rx:", 3); //???????? 
      HalUARTWrite(0, pkt->cmd.Data, pkt->cmd.DataLength); //???????????????? 
      HalUARTWrite(0, "\n", 1); //???????? 
      
      
      if(pkt->cmd.DataLength == 4)//??????????????
      {
          if((pkt->cmd.Data)[1] == '0')
          {
            Light_Work_State = 0;
            MYLED = LIGHT_PIN;
          }else if((pkt->cmd.Data)[1] == '1')  //??
          {
            Light_Work_State = 1;
            MYLED =1;
          }else if((pkt->cmd.Data)[1] == '2'){  //??
            Light_Work_State = 2;
            MYLED =0;
          }
          
          if((pkt->cmd.Data)[2] == '0')
          {
            Feng_Work_State = 0;
            FENG = MQ_PIN;
          }else if((pkt->cmd.Data)[2] == '1')  //??
          {
            Feng_Work_State = 1;
            FENG =0;
          }else if((pkt->cmd.Data)[2] == '2'){  //??
            Feng_Work_State = 2;
            FENG =1;
          }
          
          if((pkt->cmd.Data)[3] == '0')
          {
            Fans_Work_State = 0;
            FANS = !MQ_PIN;
          }else if((pkt->cmd.Data)[3] == '1')  //??
          {
            Fans_Work_State = 1;
            FANS =1;
          }else if((pkt->cmd.Data)[3] == '2'){  //??
            Fans_Work_State = 2;
            FANS =0;
          }
      }
      
      break;  
  }
}

/*********************************************************************
 * @fn      GuangApp_SendTheMessage
 *
 * @brief   Send "the" message.
 *
 * @param   none
 *
 * @return  none
 */
void GuangApp_SendTheMessage( void )
{
  
  byte temp[3], humidity[3], light_mq[4],strTemp[9]; 
  DHT11(); //?????????? 
  
    //??????????????????????????????
    if(MQ_PIN == 0)         //?????????????????? ??????????????        
    {
      if(Feng_Work_State == 0) FENG = 0;          //????????
      //if(Fans_Work_State == 0) FANS = 1;          //??????
      light_mq[0] = 0x31;
      LCD_P16x16Ch(80,2,8*16);
    }else
    {
      if(Feng_Work_State == 0) FENG = 1;         //????????
      //if(Fans_Work_State == 0) FANS = 0;          //??????
      light_mq[0] = 0x30;
      LCD_P16x16Ch(80,2,9*16);
    }
    
    if(LIGHT_PIN == 1)    //????????????????????????LED??
    {
        if(Light_Work_State == 0) MYLED =1;
        light_mq[1] = 0x31;
        LCD_P16x16Ch(0,2,9*16);   //??????????
    }
    else
    {
        if(Light_Work_State == 0) MYLED = 0;       //????????????????????LED????
        light_mq[1] = 0x30;
         LCD_P16x16Ch(0,2,8*16);   //??????????
    }
  light_mq[2] = 'e'; //????????
  light_mq[3] = '\0'; 
  //ds18b20????
 
  //????????
  if(Fans_Work_State == 0)
  {
    if( MQ_PIN == 1 && (wendu_shi<2  || (wendu_shi==2 && wendu_ge <= 6)) )
    {
      FANS = 0;
    }else{
      FANS = 1;
    }
  }
    
  //??????????????????????,?? LCD ???? 
  temp[0] = wendu_shi+0x30; 
  temp[1] = wendu_ge+0x30; 
  temp[2] = '\0'; 
  humidity[0] = shidu_shi+0x30; 
  humidity[1] = shidu_ge+0x30; 
  humidity[2] = '\0';
  //?????????????????????????????? 

  strTemp[0] = ZIGBEE_ID; 
  osal_memcpy(&strTemp[1], temp, 2); 
  osal_memcpy(&strTemp[3], humidity, 2); 
  osal_memcpy(&strTemp[5], light_mq, 4); 
  //?????????????????????????????????? 
  HalUARTWrite(0, "T&H:", 4); 
  HalUARTWrite(0, strTemp, 7); 
  HalUARTWrite(0, "\n",1); 
  
  //?????? LCD ???? 

  LCD_P8x16Str(48, 4, temp); //LCD ?????????? 
  LCD_P8x16Str(48, 6, humidity); //LCD ??????????
  

  
  //????????????
  if ( AF_DataRequest( &GuangApp_DstAddr, &GuangApp_epDesc,
                       GENERICAPP_CLUSTERID,
                       9,
                       strTemp,
                       &GuangApp_TransID,
                       AF_DISCV_ROUTE, AF_DEFAULT_RADIUS ) == afStatus_SUCCESS )
  {
    // Successfully requested to be sent.
  }
  else
  {
    // Error occurred in request to send.
  }
}

extern void rxCB(uint8 port,uint8 event)
{
  
  if (GuangApp_TxLen < GUANG_APP_TX_MAX)
  {
    GuangApp_TxLen = HalUARTRead(port, GuangApp_TxBuf, GUANG_APP_TX_MAX);
    if (GuangApp_TxLen)
    {
       if(port==1)
        {
          if (GuangApp_NwkState == DEV_ZB_COORD)
          {
            HalUARTWrite(0, GuangApp_TxBuf, GuangApp_TxLen);
            if( AF_DataRequest( &GuangApp_Broadcast,         //????????????
                       &GuangApp_epDesc,                    //??
                       GENERICAPP_BROADCASTID,       //??????
                       GuangApp_TxLen,                             // ????????????
                       GuangApp_TxBuf,                  // ??????????????
                       &GuangApp_TransID,     // ????ID??
                       AF_DISCV_ROUTE,      // ????????????????????
                       AF_DEFAULT_RADIUS ) == afStatus_SUCCESS )  //????????????????????AF_DEFAULT_RADIUS
            {
            }
          }
        }
    }            
    GuangApp_TxLen = 0;
  }
}
/*********************************************************************
*********************************************************************/
