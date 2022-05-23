#include  <stdio.h>
#include  <signal.h>

unsigned long  counter;            
int            MAX;                
int            ALARMcount;         
int            SECOND;             

/*
  NOTE: to call an alarm over and over again

in the alarm handler:

sigset_t sigset;
sigfillset(&sigset);
sigprocmask(SIG_UNBLOCK,&sigset,NULL);
*/

void  ALARMhandler(int sig)
{
     signal(SIGALRM, SIG_IGN);      
     ALARMcount++;              
     printf("*** ALARMhandler --> alarm received no. %d.\n", ALARMcount);
     printf("*** ALARMhandler --> counter = %ld\n", counter);
     printf("*** ALARMhandler --> alarm reset to %d seconds\n", SECOND);
     if (ALARMcount == MAX) {          
          printf("*** ALARMhandler --> Maximum alarm count reached.  exit\n");
          exit(0);
     }          
     counter = 0;                       /* otherwise, reset counter */
     alarm(SECOND);                     /* set alarm for next run   */
     signal(SIGALRM, ALARMhandler);     /* reinstall the handler    */
}

void  main(int argc, char *argv[])
{
     if (argc != 3) {
          printf("Use: %s seconds max-alarm-count\n", argv[0]);
          printf("No. of seconds is set to 1\n");
          SECOND = 1;
          printf("Max number of alarms set to 5\n");
          MAX    = 5;
     }
     else {
          SECOND = atoi(argv[1]);
          MAX    = atoi(argv[2]);
     }
     counter    = 0;              
     ALARMcount = 0;                  
     signal(SIGALRM,ALARMhandler);    
     printf("Alarm set to %d seconds and is ticking now.....\n", SECOND);
     alarm(SECOND);                   
     while (1)                        
          counter++;

}
