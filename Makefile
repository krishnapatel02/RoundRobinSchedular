CFLAGS = -W -Wall -I/usr/local/include -g
LDFLAGS = -L/usr/local/lib -g
PROGRAMS = thv1 thv2 thv3 thv4
OBJECTS = p1fxns.o
LIBRARIES = -lADTs

all: $(PROGRAMS)


thv1: thv1.o $(OBJECTS)
	gcc $(LDFLAGS) -o $@ $^ $(LIBRARIES)

thv2: thv2.o $(OBJECTS)
	gcc $(LDFLAGS) -o $@ $^ $(LIBRARIES)

thv3: thv3.o $(OBJECTS)
	gcc $(LDFLAGS) -o $@ $^ $(LIBRARIES)

thv4: thv4.o $(OBJECTS)
	gcc $(LDFLAGS) -o $@ $^ $(LIBRARIES)

iobound: iobound.o
	gcc $(LDFLAGS) -o $@ $^ $(LIBRARIES)

cpubound: cpubound.o
	gcc $(LDFLAGS) -o $@ $^ $(LIBRARIES)



thv1: p1fxns.h
thv2: p1fxns.h
thv3: p1fxns.h
thv4: p1fxns.h

clean:
	rm -f $(PROGRAMS) $(OBJECTS)
