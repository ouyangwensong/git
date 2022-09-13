CC = gcc
TARGET = link
src = $(wildcard ./src/*.c)
obj = $(patsubst ./src/%.c, ./obj/%.o, $(src))
INCLUDE = -I./inc/
LDFLAGS = -L ./lib
LIBS = -lasound -lfftw3f -lm



$(TARGET):$(obj)
	$(CC) $(obj) -o $(TARGET) $(LDFLAGS) $(LIBS)
	
$(obj):./obj/%.o:./src/%.c
	$(CC) $(INCLUDE) -c $^ -o $@

clean:
	-rm -rf $(target) $(obj)

.PHONY: clean ALL
