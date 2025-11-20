CC=gcc
CFLAGS=-c -Wall -g
LDFLAGS=-ljpeg
SOURCES = mandel.c jpegrw.c mandel_movie.c
OBJECTS = $(SOURCES:.c=.o)
EXECUTABLES = mandel mandel_movie

all: $(EXECUTABLES)

# pull in dependency info for *existing* .o files
-include $(OBJECTS:.o=.d)


mandel: mandel.o jpegrw.o
	$(CC) mandel.o jpegrw.o $(LDFLAGS) -o $@

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@

mandel_movie: mandel_movie.o
	$(CC) mandel_movie.o -lm -o mandel_movie

.c.o:
	$(CC) $(CFLAGS) $< -o $@
	$(CC) -MM $< > $*.d

clean:
	rm -rf $(OBJECTS) $(EXECUTABLES) *.d
