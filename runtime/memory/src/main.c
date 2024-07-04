#include <infiniband/verbs.h>
#include <stdio.h>
#include <string.h>

int main() {
  int num_devices;
  struct ibv_device** devs = ibv_get_device_list(&num_devices);
  if (!devs) {
    printf("failed to get RDMA device list: %s", strerror(errno));
    return -1;
  }
  if (num_devices <= 0) {
    printf("no RDMA device found\n");
    return -1;
  }
  for (int i = 0; i < num_devices; i++) {
    printf("found RDMA device: %s\n", devs[i]->name);
  }

  struct ibv_device* dev = devs[0];
  struct ibv_context* ctx = ibv_open_device(dev);
  if (!ctx) {
    printf("failed to open RDMA device %s", dev->name);
    return -1;
  }
  ibv_free_device_list(devs);

  printf("%s\n", ctx->device->name);

  ibv_close_device(ctx);
  return 0;
}
