# timertest

To build:

g++ ./timerfd.cc -W -O3 -g0 -o ./timerfd

To run:

Usage: ./timerfd 
[-i interval usecs,                           default 1000]
[-n num iterations,                           default 10000] 
[-r realtime priority,                        default 0] 
[-p sched policy 0 = OTHER, 1 = RR, 2 = FIFO, default 0]

This application will track the timer fire time vs the target fire time over an number of iterations specified
by the "-n" option. The timer interval can be adjusted using the "-i" option.

For "realtime" priorities "-r" other than 0 will require a schedule policy of 1 for round robin or 2 for fifo.

Example:
./timerfd -p 2 -r 10
begin 10000 iterations each 1000 usec long, est time 9.999000 sec

total time sec 9.999034, results in usec, dti min 3, max 106, mean 33.171400, stdv 5.292091

Where "dti" is the delta T for each iteration.


