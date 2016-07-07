# UnixDemo

1)  UnixSharememoryAndSemaphoreDemo

    compile cmd: gcc -o test UnixSharememoryAndSemaphoreDemo.c
    run: ./test
    check system share memory cmd: 
        ipcs -m
    remove system share memory cmd:
      ipcrm -m shmid

2)  UnixThreadDemo

    compile cmd: gcc -o test UnixThreadDemo.c -lpthread
    run: /test -s 0x100000 -n 5000
