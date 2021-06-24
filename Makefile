CXX = g++
CXXFLAGS = -O3 -Wall -std=c++17
EIGEN_INCLUDE = -I/usr/include/eigen3
OPENGL_LIBS = -lGL -lglut

LITERATE_SOURCES = marier_spheres.lilit.md
SOURCES = interpolator/marier_spheres.h examples/interpolators_demo.cpp
EXAMPLES =                              examples/interpolators_demo examples/interpolators_demo.html
DOCS_EXTRAS = docs/marier_spheres.html.header
DOCS = docs/marier_spheres.html

all: ${SOURCES} ${EXAMPLES} ${DOCS}

${SOURCES}: ${LITERATE_SOURCES}
	@echo tangling $<
	@lilit $<

examples/interpolators_demo.html: examples/interpolators_demo.cpp interpolator/marier_spheres.h
	@echo building $@
	@em++ ${CXXFLAGS} ${EIGEN_INCLUDE} -s USE_SDL=2 -s USE_SDL_TTF=2 -s ALLOW_MEMORY_GROWTH=1 -fsanitize=undefined -o examples/interpolators_demo.html examples/interpolators_demo.cpp

examples/interpolators_demo: examples/interpolators_demo.cpp interpolator/marier_spheres.h
	@echo building $@
	@g++ ${CXXFLAGS} ${EIGEN_INCLUDE} -I/usr/include/SDL2 -lSDL2 -lGLESv2 -o $@ $@.cpp

${DOCS}: ${LITERATE_SOURCES} ${DOCS_EXTRAS}
	@echo weaving $@
	@pandoc -f markdown -t html -o $@ -s --mathjax -H $@.header marier_spheres.lilit.md --metadata title="Marier Intersecting N-Spheres Interpolator"

docs: ${DOCS}

sources: ${SOURCES}

examples: ${EXAMPLES}

clean:
	@echo cleaning up
	rm -f examples/*.wasm examples/*.js examples/*.html ${EXAMPLES} interpolated_colors.bmp

.PHONY: all clean docs sources examples
