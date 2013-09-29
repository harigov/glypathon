all:
	g++ -g -Wall -W -Werror glyph_validator.cc glyph_detector.cc main.cc -o main `pkg-config opencv --cflags --libs`
