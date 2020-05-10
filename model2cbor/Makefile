TINY_OBJ_LOADER_INCLUDE_PATH = /opt/tinyobjloader/include
JSON_INCLUDE_PATH = /opt/jsonforcpp
GLM_INCLUDE_PATH = /opt/glm-0.9.9.5/glm
AMAZING_LABYRINTH = ../app/src/main/cpp

CPPFLAGS := -Wall -Werror -std=c++14 -I$(JSON_INCLUDE_PATH) -I$(AMAZING_LABYRINTH) -I$(GLM_INCLUDE_PATH) -I$(TINY_OBJ_LOADER_INCLUDE_PATH)
DBGFLAGS = -ggdb
NDBGFLAGS = -O3
OBJS = model2cbor.o modelLoader.o

all: model2cbor cbor2json

modelLoader.o : $(AMAZING_LABYRINTH)/modelLoader.cpp
	g++ $(CPPFLAGS) $(DBGFLAGS) -c -o $@ $<

%.o: %.c
	g++ $(CPPFLAGS) $(DBGFLAGS) -c -o $@ $<

model2cbor: $(OBJS)
	g++ -o model2cbor $^ /usr/lib/x86_64-linux-gnu/libcbor.so

cbor2json: cbor2json.cpp
	g++ $(CPPFLAGS) -o cbor2json $^

clean:
	rm -f model2cbor
	rm -f *.o
