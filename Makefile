$(eval OS := $(shell uname))

CXX=g++
CFLAGS=--std=c++11 -g
INC=-I/usr/local/include
SRCS=body.cpp geo.cpp camera.cpp cli.cpp form.cpp cli.cpp main.cpp
LINK=-lode

ifeq ($(OS),Darwin)
	LINK +=-lpthread -lm -lglfw3 -framework Cocoa -framework OpenGL -framework IOKit -framework CoreVideo
	LINK += -lopencv_videoio
else
	LINK +=-lglfw3 -lGL -lX11 -lXi -lXrandr -lXxf86vm -lXinerama -lXcursor -lrt -lm -pthread -ldl
	CFLAGS += -D_XOPEN_SOURCE=500 -D_GNU_SOURCE -DGL_GLEXT_PROTOTYPES
endif

obj:
	mkdir obj

obj/%.o: src/%.cpp obj
	$(CXX) $(CFLAGS) $(INC) -c $< -o $@

botshop: $(addprefix obj/,$(SRCS:.cpp=.o))
	$(CXX) $(CFLAGS) $(INC) $^ -o $@ $(LINK)

clean:
	rm -rf obj
