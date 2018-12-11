
CC=gcc
CFLAGS=-Wall
EXTRA_LIBS += -lbcm2835 -lwiringPi

EXEC=spi_run
SRC=spi_run.c data_st.c lib/list.c
$(EXEC):$(SRC)
	$(CC) $^ $(CFLAGS) -o $@ $(EXTRA_LIBS)
clean:
	rm -f *.o $(EXEC)
