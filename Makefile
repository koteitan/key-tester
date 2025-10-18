CC = gcc
CFLAGS = -Wall -Wextra -O2 -std=c99
LDFLAGS =
TARGET = key-tester
SRC = key-tester.c

.PHONY: all clean install

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC) $(LDFLAGS)

clean:
	rm -f $(TARGET)

install: $(TARGET)
	install -m 755 $(TARGET) /usr/local/bin/

debug: CFLAGS += -g -O0
debug: clean all

help:
	@echo "Available targets:"
	@echo "  make          - Build key-tester (WSL2 compatible version)"
	@echo "  make clean    - Remove built files"
	@echo "  make install  - Install to /usr/local/bin (requires sudo)"
	@echo "  make debug    - Build with debug symbols"
	@echo ""
	@echo "Usage:"
	@echo "  ./$(TARGET)"
