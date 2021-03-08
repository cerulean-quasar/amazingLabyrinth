TINY_OBJ_LOADER_INCLUDE_PATH = /opt/tinyobjloader/include
JSON_INCLUDE_PATH = /opt/jsonforcpp
GLM_INCLUDE_PATH = /opt/glm-0.9.9.5/glm
BOOST_PATH=/opt/boost_1_70_0
AMAZING_LABYRINTH = ../app/src/main/cpp

CPPFLAGS := -Wall -Werror -std=c++14 -I$(JSON_INCLUDE_PATH) -I$(AMAZING_LABYRINTH) -I$(GLM_INCLUDE_PATH) -I$(TINY_OBJ_LOADER_INCLUDE_PATH) -D"BOOST_ROOT="$(BOOST_PATH) -I$(BOOST_PATH)
DBGFLAGS = -ggdb
NDBGFLAGS = -O3
OBJS = model2cbor.o modelLoader.o

IMAGE_DIR=../imagesrc
IMAGERESULTS_DIR=../app/src/main/assets/textures
IMAGEFILES=$(shell find $(IMAGE_DIR) -type f -name "*.xcf")

MODEL_DIR=../modelsrc
MODELRESULTS_DIR=../app/src/main/assets/models
MODELFILES=$(shell find $(MODEL_DIR) -type f -name "*.obj")

CONFIG_DIR=../configsrc/levels
CONFIGRESULTS_DIR=../app/src/main/assets/configs/levels
CONFIGFILES=$(shell find $(CONFIG_DIR) -type f -name "*.json")

MODELRESULTFILES=$(foreach file,$(MODELFILES),$(subst $(MODEL_DIR),$(MODELRESULTS_DIR),$(addsuffix .modelcbor,$(basename $(file)))))
IMAGERESULTFILES=$(foreach file,$(IMAGEFILES),$(subst $(IMAGE_DIR),$(IMAGERESULTS_DIR),$(addsuffix .png,$(basename $(file)))))
CONFIGRESULTFILES=$(foreach file,$(CONFIGFILES),$(subst $(CONFIG_DIR),$(CONFIGRESULTS_DIR),$(addsuffix .cbor,$(basename $(file)))))

all: model2cbor cbor2json json2cbor $(MODELRESULTFILES) $(IMAGERESULTFILES) $(CONFIGRESULTFILES) $(CONFIGRESULTS_DIR)/../levels.json

modelLoader.o : $(AMAZING_LABYRINTH)/levelDrawer/modelTable/modelLoader.cpp
	g++ $(CPPFLAGS) $(DBGFLAGS) -c -o $@ $<

%.o: %.c
	g++ $(CPPFLAGS) $(DBGFLAGS) -c -o $@ $<

model2cbor: $(OBJS)
	g++ -o model2cbor $^ /usr/lib/x86_64-linux-gnu/libcbor.so

cbor2json: cbor2json.cpp
	g++ $(CPPFLAGS) -o cbor2json $^

json2cbor: json2cbor.cpp
	g++ $(CPPFLAGS) -o json2cbor $^

$(IMAGERESULTS_DIR)/%.png: $(IMAGE_DIR)/%.xcf
	mkdir -p $(dir $@)
	xcf2png $< -o - | pngquant --verbose --force -o $@ -

$(MODELRESULTS_DIR)/%.modelcbor: $(MODEL_DIR)/%.obj model2cbor
	mkdir -p $(dir $@)
	./model2cbor -d $(dir $@) $<

$(CONFIGRESULTS_DIR)/%.cbor: $(CONFIG_DIR)/%.json json2cbor
	mkdir -p $(dir $@)
	./json2cbor $< $@

$(CONFIGRESULTS_DIR)/../levels.json: $(CONFIG_DIR)/../levels.json
	mkdir -p $(dir $@)
	cp $< $@

clean:
	rm -f model2cbor
	rm -f cbor2json
	rm -f json2cbor
	rm -f *.o
