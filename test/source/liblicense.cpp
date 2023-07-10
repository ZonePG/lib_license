#include <doctest/doctest.h>
#include <liblicense/version.h>
#include <liblicense.h>

#include <string>

TEST_CASE("liblicense") {

}

TEST_CASE("liblicense version") {
  CHECK(std::string(LIBLICENSE_VERSION) == std::string("1.0"));
}
