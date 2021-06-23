#include "simdjson.h"
#include "test_ondemand.h"

using namespace std;
using namespace simdjson;
using error_code=simdjson::error_code;

#if SIMDJSON_EXCEPTIONS

bool basics_1() {
  TEST_START();

  ondemand::parser parser;
  auto json = padded_string::load("twitter.json");
  ondemand::document doc = parser.iterate(json); // load and parse a file

  simdjson_unused auto unused_doc = doc.get_object();

  TEST_SUCCEED();
}

bool basics_2() {
  TEST_START();

  ondemand::parser parser;
  auto json = std::string_view("[1,2,3]");
  ondemand::document doc = parser.iterate(json); // parse a string

  simdjson_unused auto unused_doc = doc.get_array();

  TEST_SUCCEED();
}

bool basics_3() {
  TEST_START();

  ondemand::parser parser;
  char json[3];
  strncpy(json, "[1]", 3);
  ondemand::document doc = parser.iterate(json, sizeof(json));

  simdjson_unused auto unused_doc = doc.get_array();

  TEST_SUCCEED();
}


bool json_array_with_array_count() {
  TEST_START();
  ondemand::parser parser;
  auto cars_json = R"( [ 40.1, 39.9, 37.7, 40.4 ] )"_padded;
  auto doc = parser.iterate(cars_json);
  auto arr = doc.get_array();
  size_t count = arr.count_elements();
  ASSERT_EQUAL(4, count);
  std::cout << count << std::endl;
  // We deliberately do it twice:
  count = arr.count_elements();
  ASSERT_EQUAL(4, count);
  std::cout << count << std::endl;
  // Next, we check whether we can iterate normally:
  std::vector<double> values(count);
  size_t index = 0;
  for(double x : arr) { values[index++] = x; }
  ASSERT_EQUAL(index, count);
  TEST_SUCCEED();
}

bool json_value_with_array_count() {
  TEST_START();
  ondemand::parser parser;
  auto cars_json = R"( {"array":[ 40.1, 39.9, 37.7, 40.4 ]} )"_padded;
  auto doc = parser.iterate(cars_json);
  auto val = doc["array"];
  size_t count = val.count_elements();
  ASSERT_EQUAL(4, count);
  std::cout << count << std::endl;
  // We deliberately do it twice:
  count = val.count_elements();
  ASSERT_EQUAL(4, count);
  std::cout << count << std::endl;
  std::vector<double> values(count);
  // Next, we check whether we can iterate normally:
  size_t index = 0;
  for(double x : val) { values[index++] = x; }
  ASSERT_EQUAL(index, count);
  TEST_SUCCEED();
}


bool json_array_count() {
  TEST_START();
  ondemand::parser parser;
  auto cars_json = R"( [ 40.1, 39.9, 37.7, 40.4 ] )"_padded;
  auto doc = parser.iterate(cars_json);
  size_t count = doc.count_elements();
  ASSERT_EQUAL(4, count);
  std::cout << count << std::endl;
  // We deliberately do it twice:
  count = doc.count_elements();
  ASSERT_EQUAL(4, count);
  std::cout << count << std::endl;
  std::vector<double> values(count);
  size_t index = 0;
  for(double x : doc) { values[index++] = x; }
  ASSERT_EQUAL(index, count);
  TEST_SUCCEED();
}

bool json_array_count_complex() {
  TEST_START();
  ondemand::parser parser;
  auto cars_json = R"( { "test":[ { "val1":1, "val2":2 }, { "val1":1, "val2":2 }, { "val1":1, "val2":2 } ] }   )"_padded;
  auto doc = parser.iterate(cars_json);
  auto test_array = doc.find_field("test").get_array();
  size_t count = test_array.count_elements();
  std::cout << "Number of elements: " <<  count << std::endl;
  size_t c = 0;
  for(ondemand::object elem : test_array) {
    std::cout << simdjson::to_string(elem);
    c++;
  }
  std::cout << std::endl;
  ASSERT_EQUAL(c, count);
  TEST_SUCCEED();

}

bool using_the_parsed_json_1() {
  TEST_START();

  try {

    ondemand::parser parser;
    auto json = std::string_view(R"(  { "x": 1, "y": 2 }  )");
    auto doc = parser.iterate(json);
    double y = doc.find_field("y"); // The cursor is now after the 2 (at })
    double x = doc.find_field("x"); // This fails, because there are no more fields after "y"

    cout << x << ", " << y << endl;
  } catch (...) {
    TEST_SUCCEED();
  }
  TEST_FAIL("expected an exception");
}

bool using_the_parsed_json_2() {
  TEST_START();

  ondemand::parser parser;
  auto json = std::string_view(R"(  { "x": 1, "y": 2 }  )");
  auto doc = parser.iterate(json);
  double y = doc["y"]; // The cursor is now after the 2 (at })
  double x = doc["x"]; // Success: [] loops back around to find "x"

  cout << x << ", " << y << endl;

  TEST_SUCCEED();
}

  bool big_integer() {
    TEST_START();
    simdjson::ondemand::parser parser;
    std::string_view docdata =  R"({"value":12321323213213213213213213213211223})";
    simdjson::ondemand::document doc = parser.iterate(docdata);
    simdjson::ondemand::object obj = doc.get_object();
    string_view token = obj["value"].raw_json_token();
    std::cout << token << std::endl;
    // token == "12321323213213213213213213213211223"
    TEST_SUCCEED();
  }

  bool big_integer_in_string() {
    TEST_START();
    simdjson::ondemand::parser parser;
    std::string_view docdata =  R"({"value":"12321323213213213213213213213211223"})";
    simdjson::ondemand::document doc = parser.iterate(docdata);
    simdjson::ondemand::object obj = doc.get_object();
    string_view token = obj["value"].raw_json_token();
    std::cout << token << std::endl;
    // token == "\"12321323213213213213213213213211223\""
    TEST_SUCCEED();
  }
bool using_the_parsed_json_3() {
  TEST_START();

  ondemand::parser parser;
  std::string_view cars_json = R"( [
    { "make": "Toyota", "model": "Camry",  "year": 2018, "tire_pressure": [ 40.1, 39.9, 37.7, 40.4 ] },
    { "make": "Kia",    "model": "Soul",   "year": 2012, "tire_pressure": [ 30.1, 31.0, 28.6, 28.7 ] },
    { "make": "Toyota", "model": "Tercel", "year": 1999, "tire_pressure": [ 29.8, 30.0, 30.2, 30.5 ] }
  ] )";

  // Iterating through an array of objects
  for (ondemand::object car : parser.iterate(cars_json)) {
    // Accessing a field by name
    cout << "Make/Model: " << std::string_view(car["make"]) << "/" << std::string_view(car["model"]) << endl;

    // Casting a JSON element to an integer
    uint64_t year = car["year"];
    cout << "- This car is " << 2020 - year << "years old." << endl;

    // Iterating through an array of floats
    double total_tire_pressure = 0;
    for (double tire_pressure : car["tire_pressure"]) {
      total_tire_pressure += tire_pressure;
    }
    cout << "- Average tire pressure: " << (total_tire_pressure / 4) << endl;
  }

  TEST_SUCCEED();
}


bool using_the_parsed_json_rewind() {
  TEST_START();

  ondemand::parser parser;
  std::string_view cars_json = R"( [
    { "make": "Toyota", "model": "Camry",  "year": 2018, "tire_pressure": [ 40.1, 39.9, 37.7, 40.4 ] },
    { "make": "Kia",    "model": "Soul",   "year": 2012, "tire_pressure": [ 30.1, 31.0, 28.6, 28.7 ] },
    { "make": "Toyota", "model": "Tercel", "year": 1999, "tire_pressure": [ 29.8, 30.0, 30.2, 30.5 ] }
  ] )";

  auto doc = parser.iterate(cars_json);
  size_t count = 0;
  for (simdjson_unused ondemand::object car : doc) {
    if(car["make"] == "Toyota") { count++; }
  }
  std::cout << "We have " << count << " Toyota cars.\n";
  doc.rewind();
  for (ondemand::object car : doc) {
    cout << "Make/Model: " << std::string_view(car["make"]) << "/" << std::string_view(car["model"]) << endl;
  }
  TEST_SUCCEED();
}

bool using_the_parsed_json_4() {
  TEST_START();

  ondemand::parser parser;
  std::string_view points_json = R"( [
      {  "12345" : {"x":12.34, "y":56.78, "z": 9998877}   },
      {  "12545" : {"x":11.44, "y":12.78, "z": 11111111}  }
    ] )";

  // Parse and iterate through an array of objects
  for (ondemand::object points : parser.iterate(points_json)) {
    for (auto point : points) {
      cout << "id: " << std::string_view(point.unescaped_key()) << ": (";
      cout << point.value()["x"].get_double() << ", ";
      cout << point.value()["y"].get_double() << ", ";
      cout << point.value()["z"].get_int64() << endl;
    }
  }

  TEST_SUCCEED();
}

bool using_the_parsed_json_5() {
  TEST_START();

  std::string_view abstract_json = R"(
    { "str" : { "123" : {"abc" : 3.14 } } }
  )";
  ondemand::parser parser;
  auto doc = parser.iterate(abstract_json);
  cout << doc["str"]["123"]["abc"].get_double() << endl; // Prints 3.14

  TEST_SUCCEED();
}

#endif // SIMDJSON_EXCEPTIONS

int using_the_parsed_json_6_process() {
  std::string_view abstract_json = R"(
    { "str" : { "123" : {"abc" : 3.14 } } }
  )";
  ondemand::parser parser;

  double value;
  auto doc = parser.iterate(abstract_json);
  auto error = doc["str"]["123"]["abc"].get(value);
  if (error) { std::cerr << error << std::endl; return EXIT_FAILURE; }
  cout << value << endl; // Prints 3.14

  return EXIT_SUCCESS;
}
bool using_the_parsed_json_6() {
  TEST_START();
  ASSERT_EQUAL(using_the_parsed_json_6_process(), EXIT_SUCCESS);
  TEST_SUCCEED();
}

int main() {
  if (
    true
#if SIMDJSON_EXCEPTIONS
//    && basics_1() // Fails because twitter.json isn't in current directory. Compile test only.
    && json_value_with_array_count()
    && json_array_with_array_count()
    && json_array_count_complex()
    && json_array_count()
    && using_the_parsed_json_rewind()
    && basics_2()
    && using_the_parsed_json_1()
    && using_the_parsed_json_2()
    && big_integer()
    && big_integer_in_string()
    && using_the_parsed_json_3()
    && using_the_parsed_json_4()
    && using_the_parsed_json_5()
#endif
    && using_the_parsed_json_6()
  ) {
    return 0;
  } else {
    return 1;
  }
}
