all:
	g++ Label.cpp Labeller.cpp -o Labeller `pkg-config --cflags --libs opencv`
