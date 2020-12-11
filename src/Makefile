objects = test.o rbtree.o

stl_rb: $(objects)
	g++ -o stl_rb $(objects) -O3 -pthread
rbtree.o:
	g++ -c rbtree.c -O3 -D _RB_RELEASE
test.o:
	g++ -c test.cpp -O3 -pthread
clean:
	rm stl_rb $(objects)
