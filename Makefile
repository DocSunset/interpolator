CXX = g++
CXXFLAGS = -O3 -Wall -std=c++17
EIGEN_INCLUDE = -I/usr/include/eigen3
OPENGL_LIBS = -lGL -lglut

LITERATE_SOURCES = lilit/*
SOURCES = include/interpolators.h demo/interpolators_demo.cpp
DEMO =                            demo/interpolators_demo demo/interpolators_demo.html
DOCS_EXTRAS = docs/interpolators.html.header
DOCS = docs/interpolators.html

all: ${SOURCES} ${DEMO} ${DOCS}

${SOURCES}: ${LITERATE_SOURCES}
	@echo tangling literate sources
	@lilit lilit/main.md

demo/interpolators_demo.html: ${SOURCES}
	@echo building $@
	@em++ ${CXXFLAGS} ${EIGEN_INCLUDE} -s USE_SDL=2 -s USE_SDL_TTF=2 -s ALLOW_MEMORY_GROWTH=1 -fsanitize=undefined -o $@ demo/interpolators_demo.cpp

demo/interpolators_demo: ${SOURCES}
	@echo building $@
	@g++ ${CXXFLAGS} ${EIGEN_INCLUDE} -I/usr/include/SDL2 -lSDL2 -lGLESv2 -o $@ $@.cpp

${DOCS}: ${LITERATE_SOURCES} ${DOCS_EXTRAS}
	@echo weaving $@
	@pandoc -f markdown -t html -o $@ -s --mathjax -H $@.header lilit/interpolators.md --metadata title="Preset Interpolation Algorithms"

docs: ${DOCS}

sources: ${SOURCES}

demo: ${DEMO}

clean:
	@echo cleaning up
	rm -f demo/*.wasm demo/*.js demo/*.html ${DEMO} interpolated_colors.bmp

.PHONY: all clean docs sources demo
