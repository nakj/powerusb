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




void send_cmd(struct libusb_device_handle *devh,int cmd)
{
  int r,i;
  uint8_t buf[64],buf2[64];
  int size=0;
  printf("send_cmd:%x\n",cmd);

  memset(buf, 0xff, sizeof(buf));
  memset(buf2, 0x00, sizeof(buf));

  buf[0] = cmd;

  r = libusb_interrupt_transfer(devh,ENDPOINT_OUT,buf, sizeof(buf),&size, 1000);
  if(r<0) {
    perror("libusb_interrupt_transfer");
    exit(1);
  }
  r = libusb_interrupt_transfer(devh,ENDPOINT_IN,buf2, sizeof(buf2),&size, 1000);

  printf("send_cmd:read:");
  for(i=0;i<2;i++){
    printf("%02x",buf2[i]);
  }
  printf("\n");

}


int main(int argc, char **argv)
{
  libusb_context *ctx = NULL;
  struct libusb_device_handle *devh = NULL;
  int r=1;


  r = libusb_init(&ctx);
  if (r < 0 ) {
    printf("usb_init:failed\n");
    exit(1);
  } else {
    libusb_set_debug(ctx,3);
    printf("init done\n");
  }  

  devh = libusb_open_device_with_vid_pid(ctx,USB_VENDOR_ID,USB_PRODUCT_ID);
  if (devh < 0 ) {
    printf("can't find PowerUSB device\n");
    goto out;

  } else {
    printf("device opened\n");
  }

  send_cmd(devh,CMD_GET_MODEL);
  send_cmd(devh,CMD_GET_FIRM_VER);
  send_cmd(devh,CMD_GET_STATE1);
  send_cmd(devh,CMD_GET_STATE2);
  send_cmd(devh,CMD_GET_STATE3);

 out:
  libusb_close(devh);
  libusb_exit(ctx);
  exit(0);
}
