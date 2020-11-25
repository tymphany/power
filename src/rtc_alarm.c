 #include <stdio.h>
 #include <linux/rtc.h>
 #include <sys/ioctl.h>
 #include <sys/time.h>
 #include <sys/types.h>
 #include <fcntl.h>
 #include <unistd.h>
 #include <stdlib.h>
 #include <errno.h>
 
 /*
  * This expects the new RTC class driver framework, working with
  * clocks that will often not be clones of what the PC-AT had.
  * Use the command line to specify another RTC if you need one.
  */
 static const char default_rtc[] = "/dev/rtc0";
 int set_alarm(int alarm_time)
 {
     int i, fd, retval, irqcount = 0;
     unsigned long tmp, data;
     struct rtc_time rtc_tm;
     const char *rtc = default_rtc;

     fd = open(rtc, O_RDONLY);
     if (fd ==  -1)
     {
         perror(rtc);
         exit(errno);
     }
     fprintf(stderr, "\n\t\t\tRTC Driver Test Example.\n\n");
 
 #if 0
     /* Turn on update interrupts (one per second) */
     retval = ioctl(fd, RTC_UIE_ON, 0);
     if (retval == -1)
     {
         if (errno == ENOTTY)
         {
             fprintf(stderr,    "\n...Update IRQs not supported.\n");
             goto test_READ;
         }
         perror("RTC_UIE_ON ioctl");
         exit(errno);
     }
     fprintf(stderr, "Counting 5 update (1/sec) interrupts from reading %s:\n",rtc);
     fflush(stderr);
     for (i=1; i<6; i++)
     {
         /* This read will block */
         retval = read(fd, &data, sizeof(unsigned long));
         if (retval == -1)
         {
             perror("read");
             exit(errno);
         }
         fprintf(stderr, " %d",i);
         fflush(stderr);
         irqcount++;
     }
     fprintf(stderr, "\nAgain, from using select(2) on /dev/rtc:");
     fflush(stderr);
     for (i=1; i<6; i++)
     {
         struct timeval tv = {5, 0};     /* 5 second timeout on select */
         fd_set readfds;
         FD_ZERO(&readfds);
         FD_SET(fd, &readfds);
         /* The select will wait until an RTC interrupt happens. */
         retval = select(fd+1, &readfds, NULL, NULL, &tv);
         if (retval == -1)
         {
             perror("select");
             exit(errno);
         }
         /* This read won't block unlike the select-less case above. */
         retval = read(fd, &data, sizeof(unsigned long));
         if (retval == -1)
         {
             perror("read");
             exit(errno);
         }
         fprintf(stderr, " %d",i);
         fflush(stderr);
         irqcount++;
     }
     /* Turn off update interrupts */
     retval = ioctl(fd, RTC_UIE_OFF, 0);
     if (retval == -1)
     {
         perror("RTC_UIE_OFF ioctl");
         exit(errno);
     }
 #endif
 //test_READ:
     /* Read the RTC time/date */
 
     printf("Set the alarm time after: ");
     //scanf("%d", &alarm_time);
     fprintf(stderr, "seconds");
     //fprintf(stderr, "Set the alarm time after %d seconds", alarm_time);
 
     retval = ioctl(fd, RTC_RD_TIME, &rtc_tm);
     if (retval == -1)
     {
         perror("RTC_RD_TIME ioctl");
         exit(errno);
     }
     fprintf(stderr, "\n\nCurrent RTC date\time is %d-%d-%d, %02d:%02d:%02d.\n",rtc_tm.tm_mday, rtc_tm.tm_mon + 1, rtc_tm.tm_year + 1900,rtc_tm.tm_hour, rtc_tm.tm_min, rtc_tm.tm_sec);
     /* Set the alarm to 5 sec in the future, and check for rollover */
     rtc_tm.tm_sec += alarm_time;
     if (rtc_tm.tm_sec >= 60)
     {
         rtc_tm.tm_sec %= 60;
         rtc_tm.tm_min++;
     }
     if  (rtc_tm.tm_min == 60)
     {
         rtc_tm.tm_min = 0;
         rtc_tm.tm_hour++;
     }
     if  (rtc_tm.tm_hour == 24)
         rtc_tm.tm_hour = 0;
     retval = ioctl(fd, RTC_ALM_SET, &rtc_tm);
     if (retval == -1)
     {
         if (errno == ENOTTY)
         {
             fprintf(stderr,"\n...Alarm IRQs not supported.\n");
             //goto test_PIE;
         }
         perror("RTC_ALM_SET ioctl");
         exit(errno);
     }
     /* Read the current alarm settings */
     retval = ioctl(fd, RTC_ALM_READ, &rtc_tm);
     if (retval == -1)
     {
         perror("RTC_ALM_READ ioctl");
         exit(errno);
     }
     fprintf(stderr, "Alarm time now set to %02d:%02d:%02d.\n",rtc_tm.tm_hour, rtc_tm.tm_min, rtc_tm.tm_sec);
     fflush(stderr);
     /* Enable alarm interrupts */
     retval = ioctl(fd, RTC_AIE_ON, 0);
     if (retval == -1)
     {
         perror("RTC_AIE_ON ioctl");
         exit(errno);
     }
 #if 0
     //fprintf(stderr, "Waiting %d seconds for alarm...", alarm_time);
     //fflush(stderr);
     ///* This blocks until the alarm ring causes an interrupt */
     //retval = read(fd, &data, sizeof(unsigned long));
     //if (retval == -1)
     //{
     //    perror("read");
     //    exit(errno);
     //}
     //irqcount++;
     //fprintf(stderr, " okay. Alarm rang.\n");
     /* Disable alarm interrupts */
     retval = ioctl(fd, RTC_AIE_OFF, 0);
     if (retval == -1)
     {
         perror("RTC_AIE_OFF ioctl");
         exit(errno);
     }
 #endif
 #if 0
 test_PIE:
     /* Read periodic IRQ rate */
     retval = ioctl(fd, RTC_IRQP_READ, &tmp);
     if (retval == -1)
     {
         /* not all RTCs support periodic IRQs */
         if (errno == ENOTTY)
         {
             fprintf(stderr, "\nNo periodic IRQ support\n");
             goto done;
         }
         perror("RTC_IRQP_READ ioctl");
         exit(errno);
     }
     fprintf(stderr, "\nPeriodic IRQ rate is %ldHz.\n", tmp);
     fprintf(stderr, "Counting 20 interrupts at:");
     fflush(stderr);
     /* The frequencies 128Hz, 256Hz, ... 8192Hz are only allowed for root. */
     for (tmp=2; tmp<=1024; tmp*=2)
     {
         retval = ioctl(fd, RTC_IRQP_SET, tmp);
         if (retval == -1)
         {
             /* not all RTCs can change their periodic IRQ rate */
             if (errno == ENOTTY)
             {
                 fprintf(stderr,"\n...Periodic IRQ rate is fixed\n");
                 goto done;
             }
             perror("RTC_IRQP_SET ioctl");
             exit(errno);
         }
         fprintf(stderr, "\n%ldHz:\t", tmp);
         fflush(stderr);
         /* Enable periodic interrupts */
         retval = ioctl(fd, RTC_PIE_ON, 0);
         if (retval == -1)
         {
             perror("RTC_PIE_ON ioctl");
             exit(errno);
         }
         for (i=1; i<21; i++)
         {
             /* This blocks */
             retval = read(fd, &data, sizeof(unsigned long));
             if (retval == -1)
             {
                 perror("read");
                 exit(errno);
             }
             fprintf(stderr, " %d",i);
             fflush(stderr);
             irqcount++;
         }
         /* Disable periodic interrupts */
         retval = ioctl(fd, RTC_PIE_OFF, 0);
         if (retval == -1)
         {
             perror("RTC_PIE_OFF ioctl");
             exit(errno);
         }
     }
 done:
 #endif
     fprintf(stderr, "\n\n\t\t\t *** Test complete ***\n");
     close(fd);
     return 0;
}