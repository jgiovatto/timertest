#include <stdlib.h>
#include <stdint.h>
#include <sched.h>
#include <stdio.h>
#include <sys/timerfd.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include <vector>
#include <limits>

// g++ ./timerfd.cc -W -O3 -g0 -o ./timerfd


int main(int argc, char *argv[])
{
    const clock_t clockType = CLOCK_REALTIME;

    // 1 msec interval timer
    uint32_t intervalUsec = 1000;

    // number of iterations
    size_t numIterations = 10000;

    int priority = 0, opt = 0;

    int policy = SCHED_OTHER;

    while((opt = getopt(argc, argv, "r:p:i:n:h")) != -1)
     {
        switch (opt) {
         case 'r':
           priority = atoi(optarg);
         break;

         case 'p':
          switch(atoi(optarg))
           {
             case 1:
               policy = SCHED_RR;
             break;

             case 2:
               policy = SCHED_FIFO;
             break;
  
             default:
              policy = SCHED_OTHER; 
           }
         break;

         case 'i':
           intervalUsec = atoi(optarg);
         break;

         case 'n':
           numIterations = atoi(optarg);
         break;

         default: 
           fprintf(stderr, "Usage: %s \n", argv[0]);
           fprintf(stderr, "[-i interval usecs,                           default 1000]\n");
           fprintf(stderr, "[-n num iterations,                           default 10000] \n");
           fprintf(stderr, "[-r realtime priority,                        default 0] \n");
           fprintf(stderr, "[-p sched policy 0 = OTHER, 1 = RR, 2 = FIFO, default 0 for -r 0, otherwise 1] \n");
           return -1;
       }
     }

    // special case no interval for a single timer
    if(numIterations == 1)
     {
       intervalUsec = 0;
     }

    // set real time priority
    if(priority > 0)
     {
      if(policy == 0)
        policy = SCHED_RR;

      const struct sched_param param = {priority};

      if(sched_setscheduler(getpid(), policy, &param) < 0)
       {
         printf("failed to set priority %s\n", strerror(errno));
         return -1;
       }
     }

    struct timeval tv0, tv1, tv;
    struct itimerspec timeout;

    std::vector<struct timeval> tv_vec;
    tv_vec.resize(numIterations);

    // create new timer, blocking
    const int fd = timerfd_create(clockType, 0);
    if(fd < 0) 
     {
        printf("failed to create timer %s\n", strerror(errno));
        return -1;
     }

    gettimeofday(&tv0, NULL);

    // set timeout initial ~2 seconds from now at the top of the second
    tv0.tv_sec += 2;
    tv0.tv_usec = 0;

    // initial
    timeout.it_value.tv_sec  = tv0.tv_sec;
    timeout.it_value.tv_nsec = 0;

    // repeat
    timeout.it_interval.tv_sec  = 0;
    timeout.it_interval.tv_nsec = intervalUsec * 1000;

    printf("begin %zu iterations each %u usec long, est time %f sec\n", 
           numIterations, 
           intervalUsec, 
           ((numIterations - 1) * intervalUsec / 1e6));

    // using absolute time
    const int ret = timerfd_settime(fd, TFD_TIMER_ABSTIME, &timeout, NULL);
    if(ret)
     {
        printf("failed to set timer %s\n", strerror(errno));
        return -1;
     }

    // the first timer should fire on the it_value time 
    for(size_t i = 0; i < numIterations; ++i) 
     {
       static uint64_t val = 0;

       if(read(fd, &val, sizeof(val)) < 0) {
         printf("read error %s\n", strerror(errno));
         return -1;
       } else {
         gettimeofday(&tv, NULL);
         tv_vec[i] = tv;
       }
    }

   gettimeofday(&tv1, NULL);

   // start time/end time in usec
   const time_t t0 = tv0.tv_sec * 1000000 + tv0.tv_usec;
   const time_t t1 = tv1.tv_sec * 1000000 + tv1.tv_usec;

   // stats in usec
   time_t min = std::numeric_limits<time_t>::max();
   time_t max = std::numeric_limits<time_t>::min();
   double sum = 0, sumsq = 0;

   std::vector<double> dti_vec;
   dti_vec.resize(numIterations);

   // find the delta_t[i] sum, min, max
   for(size_t i = 0; i < numIterations; ++i)
    {
      // t[i]
      const time_t ti = tv_vec[i].tv_sec * 1000000 + tv_vec[i].tv_usec;

      // t[i] - target time
      const time_t dti = ti - (t0 + (i * intervalUsec));

      min = std::min(min, dti);
      max = std::max(max, dti);
      sum += dti;

      dti_vec[i] = dti;
    }

   const double mean = sum / numIterations;

   for(size_t i = 0; i < numIterations; ++i)
    {
      sumsq += pow((dti_vec[i] - mean), 2);
    }
    
   const double stdv = sqrt(sumsq / numIterations);

   printf("\ntotal time sec %f, results in usec, dti min %ld, max %ld, mean %f, stdv %f\n", 
          (t1 - t0)/1e6, min, max, mean, stdv);

   return 0;
}
