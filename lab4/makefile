CC		= gcc

CFLAGS		= -g -Wall -pedantic -std=c99

OBJS		= main.o list.o error.o random.o set.o dataflow.o

OUT		= live

LDFLAGS		= -lpthread

S=100000
V=100000
U=4
A=1000
T=4
P=0

.PHONY: all run clean debug

all: debug run

debug: CFLAGS += -O3
debug: $(OUT)

$(OUT): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) $(LDFLAGS) -o $(OUT)

run: $(OUT)
	#valgrind --tool=helgrind ./$(OUT) $(S) $(V) $(U) $(A) $(T) $(P)
	./$(OUT) $(S) $(V) $(U) $(A) $(T) $(P)

clean:
	rm -f $(OUT) $(OBJS) cfg.dot
