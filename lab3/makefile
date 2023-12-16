CFLAGS = -std=c++11 -Wall -Werror -I ./interfaces/ -Wextra -pedantic-errors
PFLAGS = -pthread

all: build

build:
	@g++ $(CFLAGS) ./src/* -o set $(PFLAGS)    

clean:
	@rm -f set
