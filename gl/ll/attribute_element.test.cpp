#include <catch2/catch.hpp>
#include "test/common.h"
#include "gl/ll/attribute_element.h"

TEST_CASE("Attribute Element types are all the same size", "[gl][attribute_element]")
{
    using namespace GL::LL;

    CHECK(sizeof(AttributeElement) == sizeof(GLfloat));
    CHECK(sizeof(AttributeElement) == sizeof(GLint));
    CHECK(sizeof(AttributeElement) == sizeof(GLuint));
}
