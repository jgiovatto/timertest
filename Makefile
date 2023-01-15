
BIN = timertest
OBJ = timerfd.o

LANG=C

CFLAGS+=-W -Wall -Werror -O2

ifeq ($(DEBUG),y)
CFLAGS += -DDEBUG -g -O0
endif

LDFLAGS=-lpthread

all : $(BIN)

clean :
	rm -f $(BIN)
	rm -f *.o core *~


.PHONY : all clean

$(BIN) : $(OBJ)
	$(CXX) $(CFLAGS) -o $@ $^ $(LDFLAGS)
