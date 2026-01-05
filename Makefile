CC=gcc
CFLAGS= -Wvla -Wextra -Werror
TARGET = master
TARGET_ATOMO=atomo
TARGET_ALIMENTATORE=alimentazione
TARGET_ATTIVATORE=attivatore
OBJ=master.o utils/utils.o utils/semaphore.o
OBJ2=src/attivatore.o  utils/semaphore.o utils/message_queue.o utils/utils.o
OBJ3=src/atomo.o utils/semaphore.o utils/message_queue.o
OBJ4=src/alimentazione.o  utils/semaphore.o utils/utils.o
build: $(OBJ) $(OBJ2) $(OBJ3) $(OBJ4)
	$(CC) $(OBJ) $(CFLAGS) -o $(TARGET)
	$(CC) $(OBJ2) $(CFLAGS) -o $(TARGET_ATTIVATORE)
	$(CC) $(OBJ3) $(CFLAGS) -o $(TARGET_ATOMO)
	$(CC) $(OBJ4) $(CFLAGS) -o $(TARGET_ALIMENTATORE)

run:
	./$(TARGET)

clean:
	rm -f $(TARGET_ATTIVATORE) $(TARGET_ATOMO) $(TARGET_ALIMENTATORE) $(TARGET)
	rm -f $(OBJ) $(OBJ2) $(OBJ3) $(OBJ4)
