#include <rtthread.h>
#include <rtdevice.h>
#include "multi_button.h"
#include <w25q512.h>
static struct button btn;

int KEY_PIN;
extern int LED0_PIN;
extern int LED1_PIN;
extern int LED2_PIN;
extern int sd_ready;
static uint8_t button_read_pin(void)
{
    return rt_pin_read(KEY_PIN);
}

void button_callback(void *btn)
{
    uint32_t btn_event_val;
    static int flag_key=0;
    btn_event_val = get_button_event((struct button *)btn);

    switch(btn_event_val)
    {
    case PRESS_DOWN:
        rt_kprintf("button press down\n");
    break;

    case PRESS_UP:
        rt_kprintf("button press up\n");
    break;

    case PRESS_REPEAT:
        rt_kprintf("button press repeat\n");
    break;

    case SINGLE_CLICK:
        rt_kprintf("button single click\n");
    break;

    case DOUBLE_CLICK:
        rt_kprintf("button double click\n");
    break;

    case LONG_PRESS_START:{
      if(flag_key==0){
          rt_pin_write(LED1_PIN, PIN_HIGH);
          flag_key=1;
          if(sd_ready==1){
              /*SD数据开始传输*/
              start_record_sd();
          }
          else{
              /*W25数据开始传输*/
              start_record_w25();
          }
      }
      else{
          rt_pin_write(LED1_PIN, PIN_LOW);
          flag_key=0;
          if(sd_ready==1){
              stop_record_sd();
          }
          else{
              /*W25数据结束传输*/
              stop_record_w25();
          }
      }
    }
    break;

    case LONG_PRESS_HOLD:
        rt_kprintf("button long press hold\n");
    break;
    }
}

void btn_thread_entry(void* p)
{
    while(1)
    {
        /* 5ms */
        rt_thread_delay(RT_TICK_PER_SECOND/200);
        button_ticks();
    }
}

int multi_button_test(void)
{
    rt_thread_t thread = RT_NULL;

    /* Create background ticks thread */
    thread = rt_thread_create("btn", btn_thread_entry, RT_NULL, 10240, 10, 10);
    if(thread == RT_NULL)
    {
        return RT_ERROR;
    }
    rt_thread_startup(thread);

    /* low level drive */
    KEY_PIN = rt_pin_get("PB.12");
    rt_pin_mode  (KEY_PIN, PIN_MODE_INPUT_PULLUP);
    button_init  (&btn, button_read_pin, PIN_LOW);
//    button_attach(&btn, PRESS_DOWN,       button_callback);
//    button_attach(&btn, PRESS_UP,         button_callback);
//    button_attach(&btn, PRESS_REPEAT,     button_callback);
//    button_attach(&btn, SINGLE_CLICK,     button_callback);
//    button_attach(&btn, DOUBLE_CLICK,     button_callback);
    button_attach(&btn, LONG_PRESS_START, button_callback);
//    button_attach(&btn, LONG_PRESS_HOLD,  button_callback);
    button_start (&btn);

    return RT_EOK;
}
INIT_APP_EXPORT(multi_button_test);
