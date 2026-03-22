#include "user_TasksInit.h"
#include "cmsis_os2.h"
#include "user_AudioFbankTask.h"
#include "user_KwsInferTask.h"
#include "user_OledDisplayTask.h"
#include "user_ShowAudioTask.h"
#include "user_SpeakerTask.h"
#include "inmp441.h"
#include "uart_cli.h"
#include "speaker.h"
#include "w25qxx.h"

/* Tasks ---------------------------------------------------------------------*/
osThreadId_t         AudioFbankTaskHandle;
const osThreadAttr_t AudioFbankTask_attributes = {
    .name       = "AudioFbankTask",
    .stack_size = 128 * 20,
    .priority   = (osPriority_t)osPriorityAboveNormal,
};

osThreadId_t         KwsInferTaskHandle;
const osThreadAttr_t KwsInferTask_attributes = {
    .name       = "KwsInferTask",
    .stack_size = 128 * 16,
    .priority   = (osPriority_t)osPriorityNormal,
};

osThreadId_t         UartCliTaskHandle;
const osThreadAttr_t UartCliTask_attributes = {
    .name       = "UartCliTask",
    .stack_size = 128 * 10,
    .priority   = (osPriority_t)osPriorityBelowNormal,
};

osThreadId_t         OledDisplayTaskHandle;
const osThreadAttr_t OledDisplayTask_attributes = {
    .name       = "OledDisplayTask",
    .stack_size = 128 * 12,
    .priority   = (osPriority_t)osPriorityBelowNormal,
};

osThreadId_t         SpeakerTaskHandle;
const osThreadAttr_t SpeakerTask_attributes = {
    .name       = "SpeakerTask",
    .stack_size = 128 * 10,
    .priority   = (osPriority_t)osPriorityNormal,
};

osThreadId_t         ShowAudioTaskHandle;
const osThreadAttr_t ShowAudioTask_attributes = {
    .name       = "ShowAudioTask",
    .stack_size = 128 * 12,
    .priority   = (osPriority_t)osPriorityLow,
};


void User_Tasks_Init(void)
{
    xFbankQueue = xQueueCreate(4, sizeof(float) * 80);
    configASSERT(xFbankQueue != NULL);

    AudioFbankTaskHandle  = osThreadNew(AudioFbankTask, NULL, &AudioFbankTask_attributes);
    xAudioFbankTaskHandle = (TaskHandle_t)AudioFbankTaskHandle;

    KwsInferTaskHandle = osThreadNew(KwsInferTask, NULL, &KwsInferTask_attributes);

    /* 创建 CLI 任务：先启动中断接收，再创建任务 */
    UART_StartReceiveIT();
    UartCliTaskHandle = osThreadNew(UartCliTask, NULL, &UartCliTask_attributes);

    /* 创建 OLED 显示任务 */
    xOledEventQueue = xQueueCreate(6, sizeof(OLED_Event_t));
    configASSERT(xOledEventQueue != NULL);
    OledDisplayTaskHandle = osThreadNew(OledDisplayTask, NULL, &OledDisplayTask_attributes);

    /* 创建 Speaker 播放任务 */
    W25Q_Init();
    xSpeakerQueue = xQueueCreate(2, sizeof(Speaker_Request_t));
    configASSERT(xSpeakerQueue != NULL);
    SpeakerTaskHandle = osThreadNew(SpeakerTask, NULL, &SpeakerTask_attributes);

    xShowAudioQueue = xQueueCreate(6, sizeof(ShowAudioMessage_t));
    configASSERT(xShowAudioQueue != NULL);
    ShowAudioTaskHandle = osThreadNew(ShowAudioTask, NULL, &ShowAudioTask_attributes);
}
