CFLAGS = -std=c++17 -O2
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