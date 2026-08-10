CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -Iinclude
SRC = src/main.c src/utils.c src/auth.c src/producers.c src/categories.c src/products.c src/orders.c src/deliveries.c src/reservations.c src/losses.c src/reports.c
OBJ = $(SRC:.c=.o)
TARGET = agri_manager

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f src/*.o $(TARGET) $(TARGET).exe
