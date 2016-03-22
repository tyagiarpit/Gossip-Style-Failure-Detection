EXECUTABLE      := p4

CXX        := g++ -pthread
CC         := gcc -pthread
LINK       := g++ -fPIC -pthread

INCLUDES  += -I. -I/ncsu/gcc346/include/c++/ -I/ncsu/gcc346/include/c++/3.4.6/backward 
LIB       := -L/ncsu/gcc346/lib

default:
	$(CXX) -g -w $(EXECUTABLE).c -o $(EXECUTABLE) $(INCLUDES) $(LIB) 

p3:
	$(CXX) -g -w $(EXECUTABLE).c -o $(EXECUTABLE) $(INCLUDES) $(LIB) 

clean:
	rm -f $(EXECUTABLE) endpoints list*

