#ifndef _COMMON_H_
#define _COMMON_H_

#include <fcntl.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#define SHM_FILE "/shm_file"
#define SEM_PATH "/sem_mmap"

struct Shared {
  int count;
};

#endif
