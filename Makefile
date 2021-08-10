STB_IMAGE_INCLUDE_PATH = ../stb

CFLAGS = -std=c++2a -O2 -I$(STB_IMAGE_INCLUDE_PATH)
LDFLAGS = -lglfw -lvulkan -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi


VulkanTest: vtge.cpp
	g++ $(CFLAGS) -o VulkanTest vtge.cpp $(LDFLAGS)

.PHONY: test clean

test: VulkanTest
	./VulkanTest

clean:
	rm -f VulkanTest

debug: LDFLAGS += -g
debug: VulkanTest