#include "simdjson.h"
#include "test_ondemand.h"

using namespace simdjson;

namespace ordering_tests {
  using namespace std;

#if SIMDJSON_EXCEPTIONS

  auto json = std::string_view("{\"coordinates\":[{\"x\":1.1,\"y\":2.2,\"z\":3.3}]}");

  bool in_order_object_index() {
    TEST_START();
    ondemand::parser parser{};
    auto doc = parser.iterate(json);
    double x{0};
    double y{0};
    double z{0};
    for (ondemand::object point_object : doc["coordinates"]) {
      x += double(point_object["x"]);
      y += double(point_object["y"]);
      z += double(point_object["z"]);
    }
    return (x == 1.1) && (y == 2.2) && (z == 3.3);
  }

  bool in_order_object_find_field_unordered() {
    TEST_START();
    ondemand::parser parser{};
    auto doc = parser.iterate(json);
    double x{0};
    double y{0};
    double z{0};
    for (ondemand::object point_object : doc["coordinates"]) {
      x += double(point_object.find_field_unordered("x"));
      y += double(point_object.find_field_unordered("y"));
      z += double(point_object.find_field_unordered("z"));
    }
    return (x == 1.1) && (y == 2.2) && (z == 3.3);
  }

  bool in_order_object_find_field() {
    TEST_START();
    ondemand::parser parser{};
    auto doc = parser.iterate(json);
    double x{0};
    double y{0};
    double z{0};
    for (ondemand::object point_object : doc["coordinates"]) {
      x += double(point_object.find_field("x"));
      y += double(point_object.find_field("y"));
      z += double(point_object.find_field("z"));
    }
    return (x == 1.1) && (y == 2.2) && (z == 3.3);
  }

  bool out_of_order_object_index() {
    TEST_START();
    ondemand::parser parser{};
    auto doc = parser.iterate(json);
    double x{0};
    double y{0};
    double z{0};
    for (ondemand::object point_object : doc["coordinates"]) {
      z += double(point_object["z"]);
      x += double(point_object["x"]);
      y += double(point_object["y"]);
    }
    return (x == 1.1) && (y == 2.2) && (z == 3.3);
  }

  bool out_of_order_object_find_field_unordered() {
    TEST_START();
    ondemand::parser parser{};
    auto doc = parser.iterate(json);
    double x{0};
    double y{0};
    double z{0};
    for (ondemand::object point_object : doc["coordinates"]) {
      z += double(point_object.find_field_unordered("z"));
      x += double(point_object.find_field_unordered("x"));
      y += double(point_object.find_field_unordered("y"));
    }
    return (x == 1.1) && (y == 2.2) && (z == 3.3);
  }

  bool out_of_order_object_find_field() {
    TEST_START();
    ondemand::parser parser{};
    auto doc = parser.iterate(json);
    double x{0};
    double y{0};
    double z{0};
    for (ondemand::object point_object : doc["coordinates"]) {
      z += double(point_object.find_field("z"));
      ASSERT_ERROR( point_object.find_field("x"), NO_SUCH_FIELD );
      ASSERT_ERROR( point_object.find_field("y"), NO_SUCH_FIELD );
    }
    return (x == 0) && (y == 0) && (z == 3.3);
  }

  bool foreach_object_field_lookup() {
    TEST_START();
    ondemand::parser parser{};
    auto doc = parser.iterate(json);
    double x{0};
    double y{0};
    double z{0};
    for (auto point_object : doc["coordinates"]) {
      for (auto field : point_object.get_object()) {
        if (field.key() == "z") { z += double(field.value()); }
        else if (field.key() == "x") { x += double(field.value()); }
        else if (field.key() == "y") { y += double(field.value()); }
      }
    }
    return (x == 1.1) && (y == 2.2) && (z == 3.3);
  }

  bool use_values_out_of_order_after_array() {
    TEST_START();
    ondemand::parser parser{};
    auto doc = parser.iterate(json);
    simdjson_result<ondemand::value> x{}, y{}, z{};
    for (auto point_object : doc["coordinates"]) {
      x = point_object["x"];
      y = point_object["y"];
      z = point_object["z"];
    }
    return (double(x) == 1.1) && (double(z) == 3.3) && (double(y) == 2.2);
  }

  bool use_object_multiple_times_out_of_order() {
    TEST_START();
    ondemand::parser parser{};
    auto json2 = std::string_view("{\"coordinates\":{\"x\":1.1,\"y\":2.2,\"z\":3.3}}");
    auto doc = parser.iterate(json2);
    auto x = doc["coordinates"]["x"];
    auto y = doc["coordinates"]["y"];
    auto z = doc["coordinates"]["z"];
    return (double(x) == 1.1) && (double(z) == 3.3) && (double(y) == 2.2);
  }

#endif // SIMDJSON_EXCEPTIONS

  bool run() {
    return
#if SIMDJSON_EXCEPTIONS
           in_order_object_index() &&
           in_order_object_find_field_unordered() &&
           in_order_object_find_field() &&
           out_of_order_object_index() &&
           out_of_order_object_find_field_unordered() &&
           out_of_order_object_find_field() &&
           foreach_object_field_lookup() &&
           use_values_out_of_order_after_array() &&
           use_object_multiple_times_out_of_order() &&
#endif // SIMDJSON_EXCEPTIONS
           true;
  }

} // namespace ordering_tests


int main(int argc, char *argv[]) {
  return test_main(argc, argv, ordering_tests::run);
}
