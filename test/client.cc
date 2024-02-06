#include <fcntl.h>
#include <sys/epoll.h>
#include <sys/stat.h>
#include <unistd.h>

#include "common.h"

int main(int argc, char **argv) {
  struct Shared *ptr;
  struct stat buf;
  sem_t *mutex;
  int fd;
  int nloop;
  int i;

  fd = shm_open(SHM_FILE, O_RDWR, 0);
  fstat(fd, &buf);
  int epfd = epoll_create(5);
  if (epfd < 0) {
    perror("create epoll");
    return -1;
  }

  if (fd == -1) {
    perror("open file");
    return -1;
  }

  struct epoll_event ev;
  ev.events = EPOLLIN;
  int rc = epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev);
  if (rc < 0) {
    perror("epoll ctl add");
    return -1;
  }
  ptr = (Shared *)mmap(NULL, buf.st_size, PROT_READ | PROT_WRITE, MAP_SHARED,
                       fd, 0);
  close(fd);

  mutex = sem_open(SEM_PATH, 0);
  nloop = atoi(argv[1]);

  for (i = 0; i < nloop; i++) {
    sem_wait(mutex);
    printf("pid %d: %d\n", getpid(), ptr->count++);
    sem_post(mutex);
    sleep(1);
  }

  return 0;
}
