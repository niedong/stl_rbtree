objects = test.o rbtree.o

stl_rb: $(objects)
	g++ -o stl_rb $(objects) -pthread
rbtree.o:
	gcc -c rbtree.c -O3 -D _RB_RELEASE -Wall
test.o:
	g++ -c test.cpp -O3 -pthread -std=c++11 -Wall
clean:
	rm stl_rb $(objects)
