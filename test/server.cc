#include "common.h"

int main() {
  struct Shared *ptr;
  sem_t *mutex;
  int fd;

  shm_unlink(SHM_FILE);
  fd = shm_open(SHM_FILE, O_RDWR | O_CREAT, 0666);
  ftruncate(fd, sizeof(struct Shared));
  ptr = (Shared *)mmap(NULL, sizeof(struct Shared), PROT_READ | PROT_WRITE,
                       MAP_SHARED, fd, 0);
  close(fd);

  sem_unlink(SEM_PATH);
  mutex = sem_open(SEM_PATH, O_CREAT, 0666, 1);
  sem_close(mutex);

  pause();

  return 0;
}
