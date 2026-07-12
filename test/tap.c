#include "ether.h"
#include "util.h"
#include <errno.h>
#include <net/if.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <linux/if_tun.h>
#include <unistd.h>

#define CLONE_DEVICE "/dev/net/tun"

int
main(int argc, char *argv[])
{
  int fd;
  char *ifname;
  struct ifreq ifr;
  u_int8_t buf[2048];
  ssize_t n;

  if (argc != 2) {
    fprintf(stderr, "usage: %s <ifname>\n", argv[0]);
    return -1;
  }
  fd = open(CLONE_DEVICE, O_RDWR);
  if (fd == -1) {
    errorf("open: %s", strerror(errno));
    return -1;
  }
  ifname = argv[1];
  strncpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name) - 1);
  ifr.ifr_flags = IFF_TAP | IFF_NO_PI;
  if (ioctl(fd, TUNSETIFF, &ifr) == -1) {
    errorf("ioctl [TUNSETIFF]: %s", strerror(errno));
    close(fd);
    return -1;
  }
  infof("waiting for packets from <%s>...", ifname);
  while (1) {
    n = read(fd, buf, sizeof(buf));
    if (n == -1) {
      if (errno == EINTR) {
        continue;
      }
      errorf("recv: %s", strerror(errno));
      close(fd);
      return -1;
    }
    debugf("receive %zd bytes data", n);
    ether_print(buf, n);
  }
  close(fd);
  return 0;
}