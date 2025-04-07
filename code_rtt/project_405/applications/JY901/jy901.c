#include <jy901.h>
#include <w25q512.h>
#include <uart4_422.h>
ring_buffer ring_uart2_buff;
jy901_data  jy901;
rt_device_t uart2_dev;
struct rt_semaphore sem_uart2;
rt_thread_t uart2_th;
struct serial_configure uart2_configs = RT_SERIAL_CONFIG_DEFAULT;
struct rt_messagequeue data_901_mq;
rt_uint8_t msg_pool[2048];
char print_buffer[700];
int  cnt_record=0;
int  print_cnt=0;
int  strlen_mq_send=1;

/*函数声明*/
rt_err_t rx_callback(rt_device_t dev, rt_size_t size);
void Check_ringbuff(char *c, int timeout);
void uart2_thread_entry(void *parameter);
int flag_init=0;

extern int test_cnt;
rt_err_t  jy901_init(rt_device_t dev)
{
       rt_err_t ret = 0;
       /*初始化环形缓冲区*/
       ring_buffer_init(&ring_uart2_buff);
       /*初始化JY901通讯串口设备号*/
       uart2_dev = rt_device_find("uart2");
       if(uart2_dev == RT_NULL){
           LOG_E("rt_device_find[uart1] failed...\n");
           return -EINVAL;
       }
       /*打开串口*/
       ret = rt_device_open(uart2_dev, RT_DEVICE_OFLAG_RDWR|RT_DEVICE_FLAG_INT_RX);
       if(ret < 0){
           LOG_E("rt_device_open[uart1] failed...\n");
           rt_kprintf("ret : %d\n",ret);
           return ret;
       }
       /*配置串口属性*/
       uart2_configs.baud_rate =BAUD_RATE_921600;
       rt_device_control(uart2_dev, RT_DEVICE_CTRL_CONFIG, (void *)&uart2_configs);
       /*绑定回调函数*/
       rt_device_set_rx_indicate(uart2_dev, rx_callback);
       /*初始化信号量*/
       ret = rt_sem_init(&sem_uart2,"rx_sem", 0, RT_IPC_FLAG_FIFO);
       if(ret < 0){
           LOG_E("rt_sem_init failed[%d]...\n",ret);
           return ret;
       }
       /* 初始化消息队列 */
      rt_mq_init(&data_901_mq, "data_901_mq",
                  msg_pool,                 /* 存放消息的缓冲区 */
                  700,                    /* 一条消息的最大长度。 2组数据包*XYZ三轴数据*每个数据是float */
                  sizeof(msg_pool),         /* 存放消息的缓冲区大小 */
                  RT_IPC_FLAG_PRIO);        /* 如果有多个线程等待，按照先来先得到的方法分配消息 */

       rt_kprintf("JY901 Init ok : %d\n",ret);

       return 0;
}
/*中断回调函数
 * 将收到的数据放入环形缓冲区
 * 释放信号量
 */
rt_err_t rx_callback(rt_device_t dev, rt_size_t size)
{
    if(flag_init)/*初始化完中断函数才能开始作用，防止上电出现assert错误*/
    {
        rt_int8_t c;
        rt_device_read(uart2_dev, 0, &c, 1);
        ring_buffer_write(c, &ring_uart2_buff);
        rt_sem_release(&sem_uart2);
    }
    return RT_EOK;
}
/*判断环形缓冲区数据
 * 有数据则读数据
 * 无数据就获取信号量
 */
void Check_ringbuff(char *c, int timeout)
{
    while (1)
    {
        if (0 == ring_buffer_read((unsigned char *)c, &ring_uart2_buff))
            return;
        else
        {
            rt_sem_take(&sem_uart2, RT_WAITING_FOREVER);
        }
    }
}
/*解析JY901的串口数据，并放入全局结构体变量里面*/
extern int flag_w25;
extern int flag_sd;
extern int fd_sd;
extern int fd_w25;
void uart2_thread_entry(void *parameter)
{
    char rev_char;
    static unsigned char ucRxBuffer[250];
    static unsigned char ucRxCnt = 0;
    char str[100];
    rt_err_t mq_result=0;
    int str_len_single=0;
    rt_uint8_t event_bit=0;
    while(1)
  {
    Check_ringbuff(&rev_char,RT_WAITING_FOREVER);
    ucRxBuffer[ucRxCnt++]=rev_char;   //将收到的数据存入缓冲区中
    if (ucRxBuffer[0]!=0x55) //数据头不对，则重新开始寻找0x55数据头
    {
        ucRxCnt=0;
    }
    if (ucRxCnt==11)
    {
        switch(ucRxBuffer[1])//判断数据是哪种数据，然后将其拷贝到对应的结构体中，有些数据包需要通过上位机打开对应的输出后，才能接收到这个数据包的数据
        {
            case 0x50: {
                memcpy(&jy901.time,&ucRxBuffer[2],8);
                break;
            }
            case 0x51:{
                memcpy(&jy901.acc,&ucRxBuffer[2],8);
                event_bit|=0x01;
                break;
            }

            case 0x52:{
                memcpy(&jy901.gro,&ucRxBuffer[2],8);
                event_bit|=0x02;
                break;
            }
            case 0x53:{
                memcpy(&jy901.ang,&ucRxBuffer[2],8);
                break;
            }
            case 0x54:{
                memcpy(&jy901.mag,&ucRxBuffer[2],8);
                break;
            }
        }
        /*读取到acc和gro以后，放入到消息队列里面*/
        if(event_bit==0x03)
        {
             jy901.f_acc[0]=(float)jy901.acc.acc_x/32768*16;
             jy901.f_acc[1]=(float)jy901.acc.acc_y/32768*16;
             jy901.f_acc[2]=(float)jy901.acc.acc_z/32768*16;
             jy901.f_gro[0]=(float)jy901.gro.gro_x/32768*2000;
             jy901.f_gro[1]=(float)jy901.gro.gro_y/32768*2000;
             jy901.f_gro[2]=(float)jy901.gro.gro_z/32768*2000;
             jy901.f_temp=(float)jy901.acc.tem/100;
             sprintf(str,"%d\t%.3f\t%.3f\t%.3f\t%.3f\t%.3f\t%.3f\t%.3f\n",test_cnt,jy901.f_acc[0],
                jy901.f_acc[1],jy901.f_acc[2],jy901.f_gro[0],jy901.f_gro[1],jy901.f_gro[2],jy901.f_temp);
             if(jy901.f_temp>10&&jy901.f_temp<50&&fabs(jy901.f_gro[0])<100&&fabs(jy901.f_gro[1])<100&&fabs(jy901.f_gro[2])<100)
             {
#if 1
             if(flag_w25) {
                  test_cnt++;
                  write(fd_w25, str, strlen(str));
              //    uart4_tx_dma(str);
               }
              if(flag_sd){
                  test_cnt++;
                  write(fd_sd, str, strlen(str));
                 // uart4_tx_dma(str);
              }

#else       /*十个包发送逻辑配合消息队列,测试发现十包发送时间差距不大,消息队列加入时间优化不大*/
              str_len_single = strlen(str);
              str[str_len_single]='\0';
              test_cnt++;
              memcpy(&print_buffer[print_cnt],&str,str_len_single);
              print_cnt+=str_len_single;
              if(cnt_record==10){
                  print_buffer[print_cnt]='\0';
                  strlen_mq_send=strlen(print_buffer);
                  mq_result = rt_mq_send(&data_901_mq, &print_buffer, strlen_mq_send);
                  if ( mq_result != RT_EOK)
                  {
                      /* 消息队列满 */
                    //  rt_kprintf("send error:%d\n",mq_result);
                  }
                  cnt_record=0;
                  print_cnt=0;
              }
              else{
                  cnt_record++;
              }
#endif
             }
             event_bit=0;
        }
        ucRxCnt=0;//清空缓存区
    }
  }
}

rt_err_t  jy901_open(rt_device_t dev, rt_uint16_t oflag)
{
    rt_err_t ret;
    if(oflag!=RT_DEVICE_OFLAG_OPEN) return  RT_ERROR;
    /*创建JY901串口解析线程*/
    uart2_th = rt_thread_create("tJY901",uart2_thread_entry , NULL, 20480, 12, 5);
    ret = rt_thread_startup(uart2_th);
    if(ret < 0){
      LOG_E("thread_startup[jy901] failed...\n");
      rt_kprintf("ret : %d\n",ret);
      return ret;
    }
    return RT_EOK;
}
rt_err_t  jy901_close(rt_device_t dev)
{
    rt_err_t ret;
    ret = rt_thread_delete (uart2_th);
    if(ret < 0){
      LOG_E("rt_thread_delete[jy901] failed...\n");
      rt_kprintf("ret : %d\n",ret);
      return ret;
    }
    return RT_EOK;
}
int rt_jy901_init(void)
{
    rt_device_t jy901_dev;
    jy901_dev = rt_device_create(RT_Device_Class_Sensor, 512);

    jy901_dev->init = jy901_init;
    jy901_dev->open = jy901_open;
    jy901_dev->close = jy901_close;

    rt_device_register(jy901_dev, "jy901", RT_DEVICE_FLAG_RDWR);

    return 0;
}

INIT_BOARD_EXPORT(rt_jy901_init);


int start_901(void)
{
    rt_device_t dev_901;
    dev_901 = rt_device_find("jy901");
    rt_device_open(dev_901,RT_DEVICE_OFLAG_OPEN);
    rt_device_init(dev_901);
    flag_init =1;
    return 0;
}
INIT_PREV_EXPORT(start_901);
