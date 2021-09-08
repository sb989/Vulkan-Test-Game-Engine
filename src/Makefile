STB_IMAGE_INCLUDE_PATH = ../../stb
TINY_OBJ_INCLUDE_PATH = ../../tinyobjloader
IDIR = ../include
CFLAGS = -std=c++2a -O2 -O3 -I$(STB_IMAGE_INCLUDE_PATH) -I$(TINY_OBJ_INCLUDE_PATH) -I$(IDIR)
LDFLAGS = -lglfw -lvulkan -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi
CC=g++
ODIR = ./obj

DEPS = $(wildcard $(IDIR)/*.hpp)

_OBJ = $(patsubst %.cpp,%.o,$(wildcard *.cpp))
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

$(ODIR)/%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

VulkanTest: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

.PHONY: test clean

test: VulkanTest
	./VulkanTest

clean:
	rm -f VulkanTest
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~ 

debug: LDFLAGS += -g
debug: VulkanTest
