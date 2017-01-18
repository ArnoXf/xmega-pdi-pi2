default: pdi

OBJS=$(addprefix objs/, \
  main.o \
  pdi.o \
  nvm.o \
  ihex.o \
  errinfo.o \
)

NOPT=$(addprefix nopt/, \
  sama5d2pio.o\
)

VPATH=src
CC=arm-linux-gnueabihf-gcc
CXX=arm-linux-gnueabihf-g++

objs/%.o: %.c
	$(CC) $(CFLAGS) $< -c -o $@

objs/%.o: %.cc
	$(CXX) $(CXXFLAGS) $< -c -o $@

nopt/%.o: %.c
	$(CC) $(NOPTFLAGS) $< -c -o $@

NOPTFLAGS+=-O0 -g -std=gnu99 -Wall -Wextra -Isrc

CFLAGS+=-O3 -g -std=gnu99 -Wall -Wextra -Isrc
CXXFLAGS+=-O3 -g -std=c++0x -Wall -Wextra -Isrc

pdi: $(OBJS) $(NOPT)
	$(CXX) $(OBJS) $(NOPT) -o $@

.PHONY: clean
clean:
	-rm -f pdi objs/*.o
