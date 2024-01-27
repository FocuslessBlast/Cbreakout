main: 
	cc src/main.c -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
clean:
	rm -f a.out
