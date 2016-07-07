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
    
    help cmd: 
        #slabtop –s c   [check task_struct]
        #ulimit -u      [max user processes]
        #sysctl -a | grep kernel.pid_max        [max system processes]
        #sysctl -a | grep vm.max_map_count      [max threads of single process]
    
    ref: http://blog.chinaunix.net/uid-20662820-id-5690021.html
