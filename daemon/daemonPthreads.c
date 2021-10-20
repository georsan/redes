#include <ctype.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#ifdef __VMS
#include <unixio.h>
#else
#include <unistd.h>
#endif

#if defined (__osf__) || defined (__hpux)
#define socklen_t int
#endif

#if defined (__VMS)
typedef unsigned int socklen_t;
#endif

#ifndef PTHREAD_STACK_MIN
#define PTHREAD_STACK_MIN 65535
#endif

#ifndef FD_SET
#ifndef __DECC
struct fd_set_struct {u_char fds_bits[64/8];};
typedef struct fd_set_struct fd_set;
#endif
#define NFDBITS         sizeof(fd_set)/sizeof (u_char)
#define FD_SETSIZE      NFDBITS
#define FD_SET(n, p)    ((p)->fds_bits[(n)/NFDBITS] |= (1 << ((n) % NFDBITS)))
#define FD_CLR(n, p)    ((p)->fds_bits[(n)/NFDBITS] &= ~(1 << ((n) % NFDBITS)))
#define FD_ISSET(n, p)  ((p)->fds_bits[(n)/NFDBITS] & (1 << ((n) % NFDBITS)))
#define FD_ZERO(p)      memset((char *)(p), 0,sizeof(*(p)))
#endif

#define AUXSERVER_PORT 5020
#define CTRL_C 3

#define IAC 0xFF
// TELNET command introducer        
#define DONT 0xFE
// NOT use option                
#define DO 0xFD
// please, DO use option            
#define WONT 0xFC
// I WILL NOT use option            
#define WILL 0xFB
// I will use option                
#define SB 0xFA
// TELNET subnegotiation introducer 
#define GA 0xF9
// Go ahead                         
#define EL 0xF8
// Erase current line               
#define EC 0xF7
// Erase current character          
#define AYT 0xF6
// Are you there?                   
#define AO 0xF5
// Abort output                     
#define IP 0xF4
// Interrupt process                
#define BREAK 0xF3
// Break                          
#define DM 0xF2
// Data mark                        
#define NOP 0xF1
// NO operation                     
#define SE 0xF0
// End of sub negotiation           
#define EOR 0xEF
// End of record                    
#define SYNCH 0xEE
// Synchronize                      
#define TELOPT_BINARY 0
// DO NOT interpret data            
#define TELOPT_ECHO 1
// Echo option                      
#define TELOPT_SGA 3
// Suppress Go Ahead option         
#define TELOPT_STAT 5
// Status                           
#define TELOPT_TM 6
// Timing Mark option               
#define TELOPT_LOGOUT 0x12
// Logout                           
#define TELOPT_TTYPE 0x18
// Terminal type option             
#define TELOPT_WS 0x1F
// Negotiate about Window Size      
#define TELOPT_SPEED 0x20
// Terminal speed option            
#define TELOPT_FLOW  0x21
// Remote Flow Control              
#define TELOPT_XDISP 0x23
// X display location               
#define TELOPT_ENVIR 0x24
// Environment option               
#define END_OF_RECORD 0x19
// end-of-record option

// TELOPT_ENVIR (environment) types - see RFC 1408
#define VAR 0
#define VALUE 1
#define ESC 2
#define USERVAR 3
 
// Sub-option qualifiers                                                    
//                                                                          
#define TELQUAL_VAR 1
// Option for environment
#define TELQUAL_VALUE 0
// Option for environment           
#define TELQUAL_IS 0
// Option is                        
#define TELQUAL_SEND 1
// Send option
#define EOFREC 239
// EOR marker

#define RUNNING_DIR     "/tmp"
#define LOCK_FILE       "daemon.lock"
#if defined (__unix__) || defined (__unix)
#define LOG_FILE        "daemon.log"
#elif defined (__VMS)
#define LOG_FILE       "SYS$ERROR:"
#endif

#define LINE_WIDTH 132
#define ROOT_UID (uid_t)0
#define MAXDATASIZE 1024 
typedef struct {
   pthread_attr_t *attr;
   int s;
} arg_t;

static unsigned char Abort[]={(unsigned char)IAC,(unsigned char)IP};

void log_message(char *filename,char *message);

void cleanup(int s)
{
  char message[LINE_WIDTH];

/*
 * If given, shutdown and close sock2.
 */
  if (shutdown(s,2) < 0){
      sprintf(message, "Daemon - Socket shutdown error: %s",strerror(errno));
      log_message(LOG_FILE,message);
  }

  if (close (s)){
      sprintf(message, "Daemon - Socket close error: %s",strerror(errno));
      log_message(LOG_FILE,message);
  }

} /* end cleanup*/

void translate_received(char *in, char *out, int len){
   register char *cp=out;
   for (;len;in++,len--){
        if (!isprint((int)*in)){
           sprintf(cp,"%02X",(unsigned char)*in);
           cp +=strlen(cp);
        }
        else{
           *cp++=*in;
        }
   }
   *cp=0;
}

void finish(void *ptr){
  arg_t *arg =(arg_t *)ptr;
  
  log_message(LOG_FILE,"Closing connection");
  cleanup (arg->s) ;
  free(arg->attr);
  free(arg);
}
/*
 * This is our per socket thread.
 */
void *serve_connection (void *ptr){
  fd_set readmask;
  register arg_t *arg =(arg_t *)ptr;
  register int r;
  char message[LINE_WIDTH];
  char received[LINE_WIDTH];
  int s;
#if defined (__VMS)
  int old;
#endif
char ruta[2048]="/home/georsan/Escritorio/ejemplos/daemon/";
 // DESCRIPTOR PARA LA GESTION DEL ARCHIVO TEXTO
	FILE *ar;
  char buf[MAXDATASIZE], *d;
  pthread_cleanup_push(finish,arg);
  /*
   * By default the pthread cancelability is set to PTHREAD_CANCEL_ENABLE
   * as per Tru64 pthread_setcancelstate man. This is the case under most
   * tested *nix, including Linux.
   *
   * Unlike all tested *nix, this does not seem the case under OpenVMS IA64
   * V8.3-1H1 where the pthread_cancel(pthread_self) seems ineffective. Under
   * OpenVMS V8.3-1H1, the only way for the pthread_cancel(pthread_self) to be
   * effective is to set the setcanceltype to PTHREAD_CANCEL_ASYNCHRONOUS.
   */
#if defined (__VMS)
   if (pthread_setcanceltype (PTHREAD_CANCEL_ASYNCHRONOUS, &old) < 0){
       fprintf(stderr,"error setting PTHREAD_CANCEL_ENABLE cancelstate\n");
       exit(EXIT_FAILURE);
   }
#endif
  for (;;) {
      FD_ZERO(&readmask);
      FD_SET(arg->s,&readmask);

  /* Is the socket s readable ? If yes, the bit at position s in readmask   */
  /* will be set to one enabling us to receive data from the remote partner */

      switch(select (arg->s+1,&readmask,NULL,NULL,NULL)){
           case -1:
                sprintf(message, "Daemon - Socket select error: %s",strerror(errno));
                log_message(LOG_FILE,message);
                goto out;
           case 0 :
                log_message(LOG_FILE,"Daemon - No data received ??");
                continue;
           case 1 :
                if (FD_ISSET (arg->s,&readmask)){
                    /*
                     * Receive message from socket s
                     */
                    bzero(buf,sizeof(buf));
                    if ((r = recv(arg->s, received ,sizeof (received), 0))< 0){
                         sprintf(message, "Daemon - Socket recv error: %s",strerror(errno));
                         log_message(LOG_FILE,message);
                         goto out;
                    }
                    
                  sprintf(message, " received length ::> %d", r);
                    
                   //ABRIR EL ARCHIVO TEXTO
                    if( (ar = fopen(strcat(ruta,received), "r")) == NULL ){
                      fprintf(stderr, "Falla al abrir el archivo  \n");
                      exit(1);
                    }
                   
                    if (r){
                        strcat(message,"; data=");
                        translate_received(received,message+strlen(message),r);
                    }
                    

                    log_message(LOG_FILE,message);
                    while( (d = fgets(buf,1024,ar)) != NULL ){
	                    	write(arg->s,buf,strlen(buf));}

                    close(arg->s); 
                    
                    if (r && received[0] != CTRL_C && 
                        memcmp(received,Abort,sizeof(Abort))){
                        r = send(arg->s, received, r, 0);
                        if (r == -1){
                            sprintf(message, "Daemon - Socket send  error: %s",strerror(errno));
                            log_message(LOG_FILE,message);
                            goto out;
                        }
                           
                    }
                    
                    if (!r)
                        goto out;
                    if (r && (received[0] == CTRL_C ||
                              !memcmp(received,Abort,sizeof(Abort))))
                        pthread_cancel(pthread_self());
                }
                break;
      }/* end switch */
  } /* end for(;;) */
out:;
   pthread_cleanup_pop(1);
   return(NULL);
}

int daemon_server(int argc,char **argv)
{
  int     s;                         /* listening socket           */
  int     on = 1;                    /* used for setsockopt        */
  struct  sockaddr_in s_name;        /* Address struct for s       */
  char    message[LINE_WIDTH];       /* message buffer.            */
  socklen_t namelength;
  arg_t *arg;
  pthread_t dynthread=(pthread_t)NULL;
 

  log_message(LOG_FILE,"Daemon - beginning of the program");
  if ((s = socket (AF_INET, SOCK_STREAM, 0)) == -1)
        {
        sprintf (message,"Daemon - socket error: %s",strerror(errno));
        log_message(LOG_FILE,message);
        exit (1) ;
        }
#ifdef WITH_KEEPALIVE
  if (setsockopt(s, SOL_SOCKET, SO_KEEPALIVE, &on, sizeof (on)) < 0){
        sprintf (message,"Daemon - setsockopt error: %s",strerror(errno));
        log_message(LOG_FILE,message);
        exit (1) ;
  }
#endif
  if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &on, sizeof (on)) < 0){
        sprintf (message,"Daemon - setsockopt error: %s",strerror(errno));
        log_message(LOG_FILE,message);
        exit (1) ;
  }
  s_name.sin_family = AF_INET ;
  s_name.sin_port = htons(AUXSERVER_PORT) ;
  s_name.sin_addr.s_addr = htonl(INADDR_ANY);
  if (bind (s,(struct sockaddr *)&s_name, sizeof (s_name))<0){
      sprintf (message,"Daemon - bind error: %s",strerror(errno));
      log_message(LOG_FILE,message);
      cleanup (s) ;
      exit(1);
  }

  if (listen (s, 5) < 0){
       sprintf (message,"Daemon - listen error: %s",strerror(errno));
       log_message(LOG_FILE,message);
       cleanup (s) ;
       exit(2);
  }
  for (;;){
      namelength = sizeof (s_name);
      arg = malloc(sizeof (arg_t));
      arg->s = accept (s,(struct sockaddr *)&s_name,&namelength) ;
      if (arg->s == -1){
          sprintf (message,"Daemon - accept error: %s",strerror(errno));
          log_message(LOG_FILE,message);
          cleanup (s);
          free(arg);
          exit(2);
      }
      sprintf (message,"Daemon - Client address : %s",inet_ntoa(s_name.sin_addr)) ;
      log_message(LOG_FILE,message);
      sprintf (message,"Daemon - Client port : %d",ntohs(s_name.sin_port) ) ;
      log_message(LOG_FILE,message);
      /* Start the thread that will serve the connection */
      arg->attr = malloc(sizeof(pthread_attr_t));
      pthread_attr_init(arg->attr);
      pthread_attr_setstacksize(arg->attr,PTHREAD_STACK_MIN+BUFSIZ+10000);
      pthread_attr_setdetachstate(arg->attr,PTHREAD_CREATE_DETACHED);
      pthread_create(&dynthread,
                     arg->attr,
                     serve_connection,
                     arg);
   }
   /* Not reached */
   return 1;
} /* end daemon server */

void log_message(char *filename,char *message)
{
     FILE *logfile;

     logfile=fopen(filename,"a");
     if(!logfile) return;
     fprintf(logfile,"%s\n",message);
     fclose(logfile);
}

void signal_handler(int sig)
{
        switch(sig) {
        case SIGHUP:
                log_message(LOG_FILE,"hangup signal catched");
                break;
        case SIGTERM:
                log_message(LOG_FILE,"terminate signal catched");
                exit(0);
                break;
        }
}
void daemonize()
{
#if defined (__unix__) || defined (__unix)
   int i,lfp;
   char str[10];

   if(getppid()==1) return; /* if parent process == 1 then already a daemon */
   i=fork();
   if (i<0) exit(1); /* fork error */
   if (i>0) exit(0); /* parent exits */
   /* 
    * child (daemon) continues
    *
    * For the setuid root to work, the user from a root account MUST
    * 1/ $ chown root ./daemon_auxserver
    * 2/ $ chmod u+s ./daemon_auxserver
    *
    * If the setuid root is indeed possible, one would activate
    * daemon_auxserver from a non-root account, WITHOUT the need for sudo'ing
    * on Linux and would read:
    * $ ./daemon_auxserver
    * $ ps -ef | grep daemon_auxserver
    * UID        PID  PPID  C STIME TTY          TIME CMD
    * root      1516     1  0 21:45 ?        00:00:00 ./daemon_auxserver
    */
   setuid(ROOT_UID);
   /* continue even if we coudn't setuid root */
   errno = 0;
   setsid(); /* obtain a new process group */
   for (i=getdtablesize();i>=0;--i) close(i); /* close all descriptors */
   i=open("/dev/null",O_RDWR); dup(i); dup(i); /* handle standart I/O */
   umask((mode_t)027); /* set newly created file permissions */
   chdir(RUNNING_DIR); /* change running directory */
   lfp=open(LOCK_FILE,O_RDWR|O_CREAT,0640);
   if (lfp<0) exit(1); /* can not open */
   if (lockf(lfp,F_TLOCK,0)<0) exit(0); /* can not lock */
   /* first instance continues */
   sprintf(str,"%d\n",getpid());
   write(lfp,str,strlen(str)); /* record pid to lockfile */
   signal(SIGCHLD,SIG_IGN); /* ignore child */
   signal(SIGTSTP,SIG_IGN); /* ignore tty signals */
   signal(SIGTTOU,SIG_IGN);
   signal(SIGTTIN,SIG_IGN);
   signal(SIGHUP,signal_handler); /* catch hangup signal */
   signal(SIGTERM,signal_handler); /* catch kill signal */
#elif defined (__VMS)
   return;
#endif
}
int main(int argc, char **argv)
{
        daemonize();
        return daemon_server(argc,argv); /* run */
}

