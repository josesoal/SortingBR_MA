CC = gcc
CFLAGS = -g -Wall # flags when *compiling*
LFLAGS = -g -Wall # flags when *linking*
LIBS = -lm # math library
SOURCES = entropy.c invdist.c uf.c calc_fitness.c sort_population.c operators.c ma.c
OBJECTS = $(SOURCES:.c=.o)
EXECUTABLE = ma

all: $(EXECUTABLE)

$(EXECUTABLE) : $(OBJECTS)
	$(CC) $(LFLAGS) $(OBJECTS) -o $@ $(LIBS)

%.o:%.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(OBJECTS) $(EXECUTABLE)
