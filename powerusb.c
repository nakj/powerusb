#include <libusb-1.0/libusb.h>

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#define USB_VENDOR_ID 0x04d8
#define USB_PRODUCT_ID 0x003f
#define ENDPOINT_IN 0x81
#define ENDPOINT_OUT 0x01

#define CMD_GET_MODEL 0xaa
#define CMD_GET_FIRM_VER 0xa7
#define CMD_GET_STATE1 0xa1
#define CMD_GET_STATE2 0xa2
#define CMD_GET_STATE3 0xac

#define DEBUG

#ifdef DEBUG
#define Dprintf printf
#else
#define Dprintf(fmt, ...) while(0){}
#endif

libusb_context *ctx = NULL;
libusb_device *dev;
libusb_device **devs;
struct libusb_device_handle *devh = NULL;


uint8_t* send_cmd(struct libusb_device_handle *devh,int cmd)
{
  int i;
  uint8_t buf[64],ret[2];

  int size=0;

  Dprintf("send_cmd:%x\n",cmd);

  memset(buf, 0xff, sizeof(buf));
  buf[0] = cmd;
  
  if((libusb_interrupt_transfer(devh,ENDPOINT_OUT,buf, 
				sizeof(buf),&size, 1000)) < 0 ) {
    perror("libusb_interrupt_transfer");
    exit(1);
  }
  memset(buf, 0x00, sizeof(buf));

  if((libusb_interrupt_transfer(devh,ENDPOINT_IN,buf, 
				sizeof(buf),&size, 1000)) < 0 ) {
    perror("libusb_interrupt_transfer");
    exit(1);
  }

  Dprintf("send_cmd:read:");
  for(i=0;i<2;i++){
    ret[i] = buf[i];
    Dprintf("%02x",buf[i]);
  }
  Dprintf("\n");
  
  return ret;
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

int main(int argc, char **argv)
{
  
  initialize();
  send_cmd(devh,CMD_GET_MODEL);
  send_cmd(devh,CMD_GET_FIRM_VER);
  send_cmd(devh,CMD_GET_STATE1);
  send_cmd(devh,CMD_GET_STATE2);
  send_cmd(devh,CMD_GET_STATE3);

#if 0
  send_cmd(devh,0x42);
  send_cmd(devh,0x44);
  send_cmd(devh,0x45);

  send_cmd(devh,0x42);
  send_cmd(devh,0x44);
  send_cmd(devh,0x45);

  send_cmd(devh,0xb1);
  send_cmd(devh,0xb2);
#endif 
  finalize();


  exit(0);
}
