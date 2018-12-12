$(shell chmod -x *.c *.h)
CC=gcc
CFLAGS=-Wall
EXTRA_LIBS += -lbcm2835 -lwiringPi -lpthread

DEPEND=spi_run.c data_st.c lib/list.c lib/debug.c 

TEST_EXEC=demo_exec
TEST_SRC=spi_usr_demo.c $(DEPEND)

$(TEST_EXEC):$(TEST_SRC)
	$(CC) $^ $(CFLAGS) -o $@ $(EXTRA_LIBS)

clean:
	rm -f *.o $(EXEC) $(TEST_EXEC)
