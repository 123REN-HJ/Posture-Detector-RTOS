#include <w25q512.h>
#include <rtdevice.h>
#include <sys/errno.h>
#include "spi_flash.h"
#include "spi_flash_sfud.h"
#include <board.h>
#include <drv_spi.h>
#include <rtthread.h>
#include "dfs_fs.h"
#include <dfs_posix.h>
#include <stdio.h>
#include <string.h>
#include <uart4_422.h>

#define W25Q_SPI_BUS_NAME        "spi1"
#define W25Q_SPI_DEVICE_NAME     "spi10"
#define W25Q_SPI_DEVICE_TYPE     "W25Q512"

int sd_ready=0;
/*一般 SPI 总线命名原则为 spix， SPI 设备命名原则为 spixy ，如 spi10 表示挂载在 spi1 总线上的 0 号设备。*/
static int rt_hw_spi_flash_init(void)
{
    __HAL_RCC_GPIOB_CLK_ENABLE();
    //此处为片选引脚
    rt_hw_spi_device_attach(W25Q_SPI_BUS_NAME, W25Q_SPI_DEVICE_NAME, GPIOA, GPIO_PIN_4);

    //flash名称要正确
    if (RT_NULL == rt_sfud_flash_probe(W25Q_SPI_DEVICE_TYPE, W25Q_SPI_DEVICE_NAME))
    {
        return -RT_ERROR;
    };

    return RT_EOK;
}
INIT_COMPONENT_EXPORT(rt_hw_spi_flash_init);

/*因为dfs_fs.c使用了mount_table系统会自动挂载，下面这个可以不用了*/
static int w25_mnt_init(void)
{
   // dfs_mkfs("elm",W25Q_SPI_DEVICE_TYPE);//格式化文件内容
    //挂载文件系统，参数：块设备名称、挂载目录、文件系统类型、读写标志、私有数据0
    if(dfs_mount(W25Q_SPI_DEVICE_TYPE,"/","elm",0,0)==0)
    {
        rt_kprintf("w25 dfs mount success\r\n");
    }
    else
    {
        rt_kprintf("w25 dfs mount failed\r\n");
    }
    return 0;
}

int sd_mnt_init(void)
{
  //  dfs_mkfs("elm",W25Q_SPI_DEVICE_TYPE);//格式化文件内容
    //挂载文件系统，参数：块设备名称、挂载目录、文件系统类型、读写标志、私有数据0
    if(dfs_mount("sd0","/sd","elm",0,0)==0)
    {
        rt_kprintf("sd dfs mount success\r\n");
        sd_ready=1;
    }
    else
    {
        rt_kprintf("sd dfs mount failed\r\n");
        sd_ready=0;
    }
    return 0;
}
MSH_CMD_EXPORT(sd_mnt_init,mount sd dfs);
MSH_CMD_EXPORT(w25_mnt_init,mount w25 dfs);

static int w25_unmnt_init(void)
{
   // dfs_mkfs("elm",W25Q_SPI_DEVICE_TYPE);//格式化文件内容
    //挂载文件系统，参数：块设备名称、挂载目录、文件系统类型、读写标志、私有数据0
    if(dfs_unmount("/")==0)
    {
            rt_kprintf("w25 dfs unmount sucess \n");
    }
    else
    {
        rt_kprintf("w25 dfs unmount failed\r\n");
    }
    return 0;
}
static int sd_unmnt_init(void)
{
  //  dfs_mkfs("elm",W25Q_SPI_DEVICE_TYPE);//格式化文件内容
    //挂载文件系统，参数：块设备名称、挂载目录、文件系统类型、读写标志、私有数据0
    if(dfs_unmount("/sd")==0)
    {
        rt_kprintf("sd dfs unmount success\r\n");
    }
    else
    {
        rt_kprintf("sd dfs unmount failed\r\n");
    }
    return 0;
}
MSH_CMD_EXPORT(sd_unmnt_init,mount sd dfs);
MSH_CMD_EXPORT(w25_unmnt_init,mount w25 dfs);
/**********************以上是初始化代码,下面是应用代码****************************************************/

extern struct rt_messagequeue data_901_mq;
rt_thread_t th_wq;


int test_cnt;
int flag_w25=0;
int flag_sd=0;
int w25_file_count=0;
int sd_file_count=0;
int fd_sd=0;
int fd_w25;
extern int  strlen_mq_send;
static void thread_wait_wq(void *parameter)
{
    rt_uint8_t recv[700]={0};

    while(1)
    {
        if (rt_mq_recv(&data_901_mq, &recv, strlen_mq_send, RT_WAITING_FOREVER) == RT_EOK)
        {
//               if(flag_w25) {
//                   write(fd_w25, recv, strlen_mq_send);
//               }
//               if(flag_sd){
//                   write(fd_sd, recv,strlen_mq_send);
//               }

        }
    }
}
int thread_sd_w25(void)
{
    rt_err_t ret=0;
    th_wq = rt_thread_create("save_th",thread_wait_wq , NULL, 20480, 11, 5);
        ret = rt_thread_startup(th_wq);
        if(ret < 0){
          LOG_E("thread_save[sd/w25] failed...\n");
          rt_kprintf("ret : %d\n",ret);
          return ret;
        }
        return RT_EOK;
}
INIT_APP_EXPORT(thread_sd_w25);

void check_dir(void)
{
    DIR *dirp;
    int ret=0;
    dirp = opendir("w25q512");
    if(dirp == RT_NULL)//创建目录
    {
        rt_kprintf("no find w25q512 DIR\n");
        ret = mkdir("/w25q512", 0x777);
        if(ret <0)
        {
            rt_kprintf("dir error\n");
        }
        else {
            rt_kprintf("creat w25q512 DIR ok\n");
        }
    }
    else {
        rt_kprintf("find w25q512 DIR\n");
    }
    closedir(dirp);
    dirp = opendir("sd");
    if(dirp == RT_NULL)//创建目录
    {
        rt_kprintf("no find sd DIR\n");
        ret = mkdir("/sd", 0x777);
        if(ret <0)
        {
            rt_kprintf("creat sd DIR fail\n");
        }
        else {
            rt_kprintf("creat sd DIR ok\n");
        }
    }
    else {
            rt_kprintf("find sd DIR\n");
    }
        closedir(dirp);
}
INIT_APP_EXPORT(check_dir);

void start_record_w25(void)
{
    rt_uint8_t recv[80]={0};
    DIR  *dir;
    struct dirent *ptr;
    char FileName[30];
    /*读取文件最后编号*/
    dir = opendir("/w25q512");
    strcpy(FileName,"w25_sen_0.txt");
    if(dir == RT_NULL)
    {
        rt_kprintf("open directory error!\n");
    }
    else {
        while((ptr = readdir(dir)) != NULL)
            {
                 if(strstr(ptr->d_name,"w25_sen_") !=NULL)
                 {
                     strcpy(FileName,ptr->d_name);
                   rt_kprintf("found %s\n", ptr->d_name);
                 }
             }
             sscanf(FileName,"w25_sen_%d.txt",&w25_file_count);
             closedir(dir);
    }
    /*为新文件写title*/
     w25_file_count++;
    sprintf(FileName,"/w25q512/w25_sen_%d.txt",w25_file_count);
    fd_w25 = open(FileName, O_RDWR | O_CREAT |O_APPEND);
    sprintf(recv,"%s","count\taccx\taccy\taccz\tgrox\tgroy\tgroz\ttemp\n");
    write(fd_w25, recv, strlen(recv));
    recv[0]='\0';

    /*重新计数*/
    rt_kprintf("w25q512 write start \n");
    /*标志位置1，在thread_wait_wq作用,开始写内存*/
    test_cnt=0;
    flag_w25=1;
}

void stop_record_w25(void)
{
    close(fd_w25);
    flag_w25=0;
    /*文件系统BUG，在写完之后close(fd_w25);以后必须重新挂载一下否则文件系统自己就崩了*/
    if(dfs_unmount("/")!=0)
    {
        rt_kprintf("dfs_unmount fail \n");
    }
    w25_mnt_init();
    rt_kprintf("w25q512 write stop...file-cnt:%d   info-cnt:%d\n",w25_file_count,test_cnt);
}

void start_record_sd(void)
{
    rt_uint8_t recv[80]={0};
    DIR  *dir;
    struct dirent *ptr;
    char FileName[30];
    /*读取文件最后编号*/
    dir = opendir("/sd");
    strcpy(FileName,"sd_sen_0.txt");
    if(dir == RT_NULL)
    {
        rt_kprintf("open directory error!\n");
    }
    else {
        while((ptr = readdir(dir)) != NULL)
            {
                 if(strstr(ptr->d_name,"sd_sen_") !=NULL)
                 {
                     strcpy(FileName,ptr->d_name);
                   rt_kprintf("found %s\n", ptr->d_name);
                 }
             }
             sscanf(FileName,"sd_sen_%d.txt",&sd_file_count);
             closedir(dir);
    }
    /*为新文件写title*/
     sd_file_count++;
    sprintf(FileName,"/sd/sd_sen_%d.txt",sd_file_count);
    fd_sd = open(FileName, O_RDWR | O_CREAT |O_APPEND);
    sprintf(recv,"%s","count\taccx\taccy\taccz\tgrox\tgroy\tgroz\ttemp\n");
    write(fd_sd, recv, strlen(recv));
    recv[0]='\0';
    /*标志位置1，在thread_wait_wq作用,开始写内存*/
    rt_kprintf("sd write start \n");
    /*重新计数*/
     test_cnt=0;
     flag_sd=1;
}

void stop_record_sd(void)

{
    rt_kprintf("sd write stop...file-cnt:%d   info-cnt:%d\n",sd_file_count,test_cnt);
    close(fd_sd);
    flag_sd=0;
    /*文件系统BUG，在写完之后close(fd_wsd);以后必须重新挂载一下否则文件系统自己就崩了*/
    sd_unmnt_init();
    sd_mnt_init();

}
MSH_CMD_EXPORT(start_record_w25,start w25);
MSH_CMD_EXPORT(stop_record_w25,stop w25);
MSH_CMD_EXPORT(start_record_sd,start sd);
MSH_CMD_EXPORT(stop_record_sd,stop sd);
