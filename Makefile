CFLAGS = -Wall

ifeq ($(DEBUG), 1)
	CFLAGS += -O0 -ggdb
else
	CFLAGS += -O2 
endif

ifeq ($(PROFILE), 1)
	CFLAGS += -fno-omit-frame-pointer
endif

LIBS = -lpcap -lprotobuf-c

OBJ = defaultdict.o client.o radiotap.o ieee80211.o tracking.o channel.o utils.o report.o report.pb-c.o
OBJPROTO = report.pb-c.c

all: client

clean:
	rm -f *.o *.pb-c.c *.pb-c.h client

%.pb.h %.pb-c.c : %.proto
	protoc-c -I. --c_out=. $<

%.o : %.c %.pb-c.c
	$(CC) -c $(INCLUDES) $(CFLAGS) $<

client: $(OBJPROTO) $(OBJ) 
	$(CC) $(OBJ) $(LIBS) $(CFLAGS) -o client
