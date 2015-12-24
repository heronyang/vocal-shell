CC = gcc -g -Wall
CFLAGS = `pkg-config --cflags pocketsphinx sphinxbase`
LDFLAGS = `pkg-config --libs pocketsphinx sphinxbase`
DMODELDIR = `pkg-config --variable=modeldir pocketsphinx`

DEPS = continuous.h
OBJS = main.o continuous.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS) -DMODELDIR=\"$(DMODELDIR)\"

main: $(OBJS)
    $(CC) $(CFLAGS) -o main -DMODELDIR=\"$(DMODELDIR)\" $(OBJS) $(LDFLAGS)

clean:
	rm -f *.o *~ main

