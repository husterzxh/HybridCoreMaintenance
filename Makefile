main:main.cpp
	g++ -std=c++11 -O3 -o main main.cpp -lpthread

.PHONY:clean
clean:
	rm main
