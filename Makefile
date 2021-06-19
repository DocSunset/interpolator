CXX = g++
CXXFLAGS = -O3 -Wall -std=c++17
EIGEN_INCLUDE = -I/usr/include/eigen3
OPENGL_LIBS = -lGL -lglut

EXAMPLES = examples/color_image examples/interactive_colors
LITERATE_SOURCES = marier_spheres.lilit
SOURCES = interpolator/marier_spheres.h examples/color_image.cpp examples/interactive_colors.cpp
DOCS_EXTRAS = docs/marier_spheres.html.header
DOCS = docs/marier_spheres.html

all: ${SOURCES} ${EXAMPLES} ${DOCS}

${SOURCES}: ${LITERATE_SOURCES}
	@echo tangling $<
	@lilit $<

examples/color_image: examples/color_image.cpp interpolator/marier_spheres.h
	@echo building $@
	@${CXX} ${CXXFLAGS} ${EIGEN_INCLUDE} -o $@ $@.cpp

examples/interactive_colors: examples/color_image.cpp interpolator/marier_spheres.h
	@echo building $@
	@${CXX} ${CXXFLAGS} ${EIGEN_INCLUDE} ${OPENGL_LIBS} -o $@ $@.cpp

${DOCS}: ${LITERATE_SOURCES} ${DOCS_EXTRAS}
	@echo weaving $@
	@pandoc -f markdown -t html -o $@ -s --mathjax -H $@.header marier_spheres.lilit --metadata title="Marier Intersecting N-Spheres Interpolator"

docs: ${DOCS}

sources: ${SOURCES}

examples: ${EXAMPLES}

clean:
	@echo cleaning up
	rm -f ${EXAMPLES} ${DOCS} interpolated_colors.bmp

.PHONY: all clean docs sources examples
