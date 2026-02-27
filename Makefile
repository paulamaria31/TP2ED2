CC = gcc

CFLAGS = -Wall -g

OBJ = main.o ordenaF1.o ordena2F.o ordenaQuick.o

TARGET = ordena

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o $(TARGET)