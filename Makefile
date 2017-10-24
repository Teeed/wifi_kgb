CFLAGS = -Wall

ifeq ($(DEBUG), 1)
	CFLAGS += -O0 -ggdb
else
	CFLAGS += -O2 
endif

ifeq ($(PROFILE), 1)
	CFLAGS += -fno-omit-frame-pointer
endif

LIBS = -lpcap

OBJ = defaultdict.o client.o radiotap.o ieee80211.o tracking.o channel.o utils.o 

all: client

clean:
	rm -f *.o client
.c.o:
	$(CC) -c $(INCLUDES) $(CFLAGS) $<

client: $(OBJ)
	$(CC) $(OBJ) $(LIBS) $(CFLAGS) -o client