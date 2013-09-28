all:
	g++ -g main.cc -o main `pkg-config opencv --cflags --libs`
