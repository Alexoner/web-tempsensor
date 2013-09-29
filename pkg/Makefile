CC=clang
CFLAGS=-lftd2xx -lm -L/usr/local/lib -g -Wall

sensor.cgi:sensor.c sensor.cgi.c
	$(CC) -o $@ $^ $(CFLAGS)
test:test.c tempsensor.c
	$(CC) -o $@ $^ $(CFLAGS)

clean:
	rm *.o sensor sensor.cgi test
