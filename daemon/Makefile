CC  =   gcc
CFLAGS  = 
OTHERFLAGS = -g

all: daemon client

daemon: daemonPthreads.c
	$(CC)  $(CFLAGS) $(OTHERFLAGS) daemonPthreads.c -lpthread -o demonio

client: client.c
	$(CC)  $(CFLAGS) $(OTHERFLAGS) client.c -o client

