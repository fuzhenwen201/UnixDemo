# UnixSharememoryAndSemaphoreDemo
this is a shm&amp;sem demo for learning unix sharememory&amp;semaphore using in multiprocess

compile cmd: gcc -o test UnixSharememoryAndSemaphoreDemo.c

run: ./test

check system share memory cmd: 
  ipcs -m

remove system share memory cmd:
  ipcrm -m shmid
