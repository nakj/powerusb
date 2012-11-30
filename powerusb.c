#include <libusb-1.0/libusb.h>

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#define USB_VENDOR_ID	0x04d8
#define USB_PRODUCT_ID	0x003f
#define ENDPOINT_IN	0x81
#define ENDPOINT_OUT	0x01

#define CMD_GET_MODEL		0xaa
#define CMD_GET_FIRM_VER	0xa7
#define CMD_GET_STATE1		0xa1
#define CMD_GET_STATE2		0xa2
#define CMD_GET_STATE3		0xac
#define CMD_GET_POWER		0xb1
#define CMD_PING		0xb2
#define CMD_OUTLET1_ON		0x41
#define CMD_OUTLET1_OFF		0x42
#define CMD_OUTLET2_ON		0x43
#define CMD_OUTLET2_OFF		0x44
#define CMD_OUTLET3_ON		0x45
#define CMD_OUTLET3_OFF		0x50

//#define DEBUG

#ifdef DEBUG
#define Dprintf printf
#else
#define Dprintf(fmt, ...) while(0){}
#endif

libusb_context *ctx = NULL;
libusb_device *dev;
libusb_device **devs;
struct libusb_device_handle *devh = NULL;
int state[4];

void usage(void)
{
  const char usage_string[] = 
    "command usage\n"
    "\tpowerusb get power [sec]\n"
    "\tpowerusb get state [1-3]\n"
    "\tpowerusb set [1-3] [on|off]\n";

  printf("%s",usage_string);
  exit(1);
}


int send_cmd(struct libusb_device_handle *devh,int cmd, uint8_t ret[2])
{
  int i;
  uint8_t buf[64];

  int size=0;

  Dprintf("send_cmd:%x\n",cmd);

  memset(buf, 0xff, sizeof(buf));
  buf[0] = cmd;
  /* packet send*/
  if((libusb_interrupt_transfer(devh,ENDPOINT_OUT,buf, 
				sizeof(buf),&size, 1000)) < 0 ) {
    perror("libusb_interrupt_transfer");
    exit(1);
  }
  memset(buf, 0x00, sizeof(buf));
  /* packet recieve if needed */
  if (cmd > 0x50) {
    Dprintf("recieve\n");
    if((libusb_interrupt_transfer(devh,ENDPOINT_IN,buf, 
				  sizeof(buf),&size, 1000)) < 0 ) {
      perror("libusb_interrupt_transfer");
      exit(1);
    }
  }
  Dprintf("send_cmd:read:");
  for(i=0;i<2;i++){
    ret[i] = buf[i];
    Dprintf("%02x",buf[i]);
  }
  Dprintf("\n");
  
  return 0;
}

void finalize(void){
  libusb_close(devh);
  libusb_exit(ctx);
}

int initialize(void){
  int r=1;
  int i=0;
  int cnt=0;
  /* libusb initialize*/
  if ((r = libusb_init(&ctx)) < 0) {
    perror("libusb_init\n");
    exit(1);
  } else {
    libusb_set_debug(ctx,3);
    Dprintf("init done\n");
  }  
  
  /* confirm powerusb device */
  /* list up all usb devices */
  if((libusb_get_device_list(ctx,&devs)) < 0) {
    perror("no usb device found");
    exit(1);
  }
  /* check every usb devices */
  while((dev =devs[i++]) != NULL) {
    struct libusb_device_descriptor desc;
    if (libusb_get_device_descriptor(dev,&desc) < 0) {
      perror("failed to get device descriptor\n");
      return 1;
    }
    /* count how many PowerUSB device connected */
    if (desc.idVendor == USB_VENDOR_ID &&
	desc.idProduct == USB_PRODUCT_ID) {
      cnt++;
      Dprintf("PowerUSB device found\n");
    }
  }
  /* no PowerUSB found*/
  if (cnt == 0) {
    fprintf(stderr, "Power USB device not connected\n");
    exit(1);
  }
  /* multi-PowerUSB device found: return error*/
  if (cnt > 1) {
    /* FIXME */
    fprintf(stderr, "multi PowerUSB is not implemented yet\n");
    exit(1);
  }


  /* open powerusb device */
  if ((devh = libusb_open_device_with_vid_pid(ctx,USB_VENDOR_ID,
					      USB_PRODUCT_ID)) < 0 ) {
    perror("can't find PowerUSB device\n");
    finalize();
    exit(1);
  } else {
    Dprintf("PowerUSB device opened\n");
  }

  /* detach kernel driver if attached. */
  /* is kernel driver active?*/
  r = libusb_kernel_driver_active(devh,0);
  if (r == 1) {
    /*detaching kernel driver*/
    r = libusb_detach_kernel_driver(devh,0);
    if (r != 0) {
      perror("detaching kernel driver failed");
      exit(1);
    }
  }
  return 0;
}

int get_status(int a)
{
  uint8_t ret[2];

  if (( a < 0) | (a > 3))
    usage();

  switch(a){
  case 1:
    send_cmd(devh,CMD_GET_STATE1,ret);
    break;
  case 2:
    send_cmd(devh,CMD_GET_STATE2,ret);
    break;
  case 3:
    send_cmd(devh,CMD_GET_STATE3,ret);
    break;
  default:
    usage();
  }
  printf("Outlet%d status:",a);
  if (ret[0] == 0){
    printf("OFF\n");
  }else {
    printf("ON\n");
  }
  return 0;
}

void get_power(int psec)
{
  uint8_t ret[2];
  char buf[5],*e;

  while (psec > 0){
    ret[0]=ret[1]=0;
    send_cmd(devh,CMD_GET_POWER,ret);
    sprintf(buf,"0x%02x%02x",ret[0],ret[1]);
    Dprintf("ret:%s\n",buf);
    printf("Current %ldmA\n",strtol(buf,&e,16));
    send_cmd(devh,CMD_PING,ret);
    sleep(1);
    psec--;
  }

}

void cmd_get(int argc,char **argv)
{
  int psec;

  if (argc < 3) 
    usage();

  if (!strcmp(argv[2],"power")) {
    Dprintf("power\n");
    if (argc  < 4 ) {
      psec =10;
    } else {
      psec = atoi(argv[3]);
    }
    get_power(psec);

    
  } else if (!strcmp(argv[2],"state")){
    Dprintf("state");
    if (argc < 4)
      usage();

    if (!strcmp(argv[3],"all")) {
      get_status(1);
      get_status(2);
      get_status(3);

    }else if (!(!strcmp(argv[3],"1") | !strcmp(argv[3],"2") | !strcmp(argv[3],"3" ))){
      usage();
    }else {
      get_status(atoi(argv[3]));
    }
  }else{
    usage();
  }
}

void cmd_set2(int s, char* cmd)
{
  uint8_t ret[2];
  int cond;
  printf("to swich condition Outlet%d:%s\n",s, cmd);

  if (!strcmp(cmd,"on")) {
    cond = 1;
  }else if (!strcmp(cmd,"off")) {
    cond = 0;
  }

  switch(s){
  case 1:
    if (cond == 0){
      send_cmd(devh,CMD_OUTLET1_OFF,ret);
    }else if (cond == 1) {
      send_cmd(devh,CMD_OUTLET1_ON,ret);
    }
    break;
  case 2:
    if (cond == 0){
      send_cmd(devh,CMD_OUTLET2_OFF,ret);
    }else if (cond == 1) {
      send_cmd(devh,CMD_OUTLET2_ON,ret);
    }
    break;
  case 3:
    if (cond == 0){
      send_cmd(devh,CMD_OUTLET3_OFF,ret);
    }else if (cond == 1) {
      send_cmd(devh,CMD_OUTLET3_ON,ret);
    }

    break;
  default:
    usage();
    exit(1);
    
  }


}

void cmd_set(int argc,char **argv)
{
  int s;
  if (argc < 4)
    usage();

  s = atoi(argv[2]);
  if ((s ==1) | (s ==2) | (s ==3)) {
    /* set routine */
    if (!strcmp(argv[3],"on")|!strcmp(argv[3],"off"))
      cmd_set2(s,argv[3]);
    else
      usage();

  } else {
    usage();
  }

}



int main(int argc, char **argv)
{
  uint8_t ret[2];
  int i;
 
  initialize();
  send_cmd(devh,CMD_GET_MODEL,ret);
  printf("Model:");
  switch (ret[0]){
  case 1:
    printf("Basic\n");
    break;
  case 2:
    printf("digIO\n");
    break;
  case 3:
    printf("watchdog\n");
    break;
  case 4:
    printf("smart\n");
    break;
  }
  send_cmd(devh,CMD_GET_FIRM_VER,ret);
  printf("Firmware version: %d.%d\n",ret[0],ret[1]);

  send_cmd(devh,CMD_GET_STATE1,ret);
  state[1] = ret[0];

  send_cmd(devh,CMD_GET_STATE2,ret);
  state[2] = ret[0];

  send_cmd(devh,CMD_GET_STATE3,ret);
  state[3] = ret[0];

  for (i=1;i<4;i++){
    printf("Outlet%d:",i);
    switch(state[i]){
    case 0:
      printf("off\n");
      break;
    case 1:
      printf("on\n");
      break;
    }

  }  

  if (argc < 2) {
    usage();
    exit(1);
  }
  if (!strcmp(argv[1],"get")) {
      cmd_get(argc,argv);
  }else if (!strcmp(argv[1],"set")) {
      Dprintf("set\n");
      cmd_set(argc,argv);
  }else {
    usage();
    exit(1);
  }


  finalize();
  exit(0);
}
