CC = gcc
CFLAGS = -Ilib/glad/include -Ilib/glfw/include -Isrc -Wall
LDFLAGS = -Llib/glfw/build/src -lglfw -lGL -ldl -lm

SRC = src/main.c src/gfx/window.c lib/glad/src/glad.c
OBJ = $(SRC:.c=.o)
TARGET = minecraft_clone

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET) $(LDFLAGS)

clean:
	rm -f $(SRC:.c=.o) $(TARGET)