CXX = g++
CXXFLAGS = -O3 -Wall -std=c++17
EIGEN_INCLUDE = -I/usr/include/eigen3
OPENGL_LIBS = -lGL -lglut

EXAMPLES = examples/color_image examples/interactive_colors
SOURCES = interpolator/marier_spheres.h examples/color_image.cpp examples/interactive_colors.cpp
DOCS = docs/marier_spheres.html

all: ${SOURCES} ${EXAMPLES} ${DOCS}

${SOURCES}: marier_spheres.lilit
	@echo tangling source
	@lilit marier_spheres.lilit

examples/color_image: examples/color_image.cpp interpolator/marier_spheres.h
	@echo building $@
	@${CXX} ${CXXFLAGS} ${EIGEN_INCLUDE} -o $@ $@.cpp

examples/interactive_colors: examples/color_image.cpp interpolator/marier_spheres.h
	@echo building $@
	@${CXX} ${CXXFLAGS} ${EIGEN_INCLUDE} ${OPENGL_LIBS} -o $@ $@.cpp

${DOCS}: marier_spheres.lilit docs/marier_spheres.html.header
	pandoc -f markdown -t html -o $@ -s --mathjax -H $@.header marier_spheres.lilit

docs: ${DOCS}

sources: ${SOURCES}

examples: ${EXAMPLES}

clean:
	@echo cleaning up
	rm -f ${EXAMPLES} ${DOCS} interpolated_colors.bmp

.PHONY: all clean docs sources examples
