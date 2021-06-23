The Basics
==========

An overview of what you need to know to use simdjson, with examples.

* [Requirements](#requirements)
* [Including simdjson](#including-simdjson)
* [Using simdjson with package managers](#using-simdjson-with-package-managers)
* [Using simdjson as a CMake dependency](#using-simdjson-as-a-cmake-dependency)
* [Versions](#versions)
* [The Basics: Loading and Parsing JSON Documents](#the-basics-loading-and-parsing-json-documents)
* [Documents Are Iterators](#documents-are-iterators)
* [Using the Parsed JSON](#using-the-parsed-json)
* [C++11 Support and string_view](#c11-support-and-string_view)
* [C++17 Support](#c17-support)
* [Minifying JSON strings without parsing](#minifying-json-strings-without-parsing)
* [UTF-8 validation (alone)](#utf-8-validation-alone)
* [JSON Pointer](#json-pointer)
* [Error Handling](#error-handling)
  * [Error Handling Example](#error-handling-example)
  * [Exceptions](#exceptions)
* [Rewinding](#rewinding)
* [Direct Access to the Raw String](#direct-access-to-the-raw-string)
* [Newline-Delimited JSON (ndjson) and JSON lines](#newline-delimited-json-ndjson-and-json-lines)
* [Thread Safety](#thread-safety)
* [Standard Compliance](#standard-compliance)


Requirements
------------------

- A recent compiler (LLVM clang6 or better, GNU GCC 7.4 or better) on a 64-bit (PPC, ARM or x64 Intel/AMD) POSIX systems such as macOS, freeBSD or Linux. We require that the compiler supports the C++11 standard or better.
- Visual Studio 2017 or better under 64-bit Windows. Users should target a 64-bit build (x64) instead of a 32-bit build (x86). We support the LLVM clang compiler under Visual Studio (clangcl) as well as as the regular Visual Studio compiler. We also support MinGW 64-bit under Windows.

Including simdjson
------------------

To include simdjson, copy [simdjson.h](/singleheader/simdjson.h) and [simdjson.cpp](/singleheader/simdjson.cpp)
into your project. Then include it in your project with:

```c++
#include "simdjson.h"
using namespace simdjson; // optional
```

You can compile with:

```
c++ myproject.cpp simdjson.cpp
```

Note:
- Users on macOS and other platforms where default compilers do not provide C++11 compliant by default should request it with the appropriate flag (e.g., `c++ -std=c++17 myproject.cpp simdjson.cpp`).

Using simdjson with package managers
------------------

You can install the simdjson library on your system or in your project using multiple package managers such as MSYS2, the conan package manager, vcpkg, brew, the apt package manager (debian-based Linux systems), the FreeBSD package manager (FreeBSD), and so on. [Visit our wiki for more details](https://github.com/simdjson/simdjson/wiki/Installing-simdjson-with-a-package-manager).

Using simdjson as a CMake dependency
------------------

You can include the simdjson library as a CMake dependency by including the following lines in your `CMakeLists.txt`:

```cmake
include(FetchContent)

FetchContent_Declare(
  simdjson
  GIT_REPOSITORY https://github.com/simdjson/simdjson.git
  GIT_TAG  v0.9.3
  GIT_SHALLOW TRUE)

FetchContent_MakeAvailable(simdjson)
```

You should replace `GIT_TAG  v0.9.3` by the version you need. If you omit `GIT_TAG  v0.9.3`, you will work from the main branch of simdjson: we recommend that if you are working on production code, you always work from a release.

Elsewhere in your project, you can declare dependencies on simdjson with lines such as these:

```cmake
add_executable(myprogram myprogram.cpp)
target_link_libraries(myprogram simdjson)
```

We recommend CMake version 3.15 or better.

See [our CMake demonstration](https://github.com/simdjson/cmake_demo_single_file). It works under Linux, FreeBSD, macOS and Windows (including Visual Studio).

The CMake build in simdjson can be taylored with a few variables. You can see the available variables and their default values by entering the `cmake -LA` command.


Versions
------------------

Users are discouraged from building production code from the
project's main branch. The main branch is used for development:
it may contain new features but also additional bugs.

Users should pick a release. They should also access the
documentation matching the release that they have chosen.
Note that new features may be added over time.

Our releases are tagged using semantic versioning: the tags
are made of three numbers prefixed by the letter `v` and separated by periods.

You can always find the latest release at the following hyperlink:

https://github.com/simdjson/simdjson/releases/latest/

The archive you download at this location contains its own corresponding
documentation.

You can also choose to browse a specific version
of the documentation and the code using GitHub,
by appending the version number to the hyperlink, like so:

https://github.com/simdjson/simdjson/blob/vx.y.z/doc/basics.md

where `x.y.z` should correspond to the version number you have
chosen.

The Basics: Loading and Parsing JSON Documents
----------------------------------------------

The simdjson library offers a  tree API, which you can access by creating a
`ondemand::parser` and calling the `iterate()` method:

```c++
ondemand::parser parser;
auto json = padded_string::load("twitter.json");
ondemand::document doc = parser.iterate(json); // position a pointer at the beginning of the JSON data
```

Or by creating a padded string (for efficiency reasons, simdjson requires a string with
SIMDJSON_PADDING bytes at the end) and calling `iterate()`:

```c++
ondemand::parser parser;
auto json = "[1,2,3]"_padded; // The _padded suffix creates a simdjson::padded_string instance
ondemand::document doc = parser.iterate(json); // parse a string
```

If you have a buffer of your own with enough padding already (SIMDJSON_PADDING extra bytes allocated), you can use `padded_string_view` to pass it in:

```c++
ondemand::parser parser;
char json[3+SIMDJSON_PADDING];
strcpy(json, "[1]");
ondemand::document doc = parser.iterate(json, strlen(json), sizeof(json));
```

We recommend against creating many `std::string` or many `std::padding_string` instances in your application to store your JSON data.
Consider reusing the same buffers and limiting memory allocations.

Documents Are Iterators
-----------------------

The simdjson library relies on an approach to parsing JSON that we call "On Demand".
A `document` is *not* a fully-parsed JSON value; rather, it is an **iterator** over the JSON text.
This means that while you iterate an array, or search for a field in an object, it is actually
walking through the original JSON text, merrily reading commas and colons and brackets to make sure
you get where you are going. This is the key to On Demand's performance: since it's just an iterator,
it lets you parse values as you use them. And particularly, it lets you *skip* values you do not want
to use.

### Parser, Document and JSON Scope

Because a document is an iterator over the JSON text, both the JSON text and the parser must
remain alive (in scope) while you are using it. Further, a `parser` may have at most
one document open at a time, since it holds allocated memory used for the parsing.

During the `iterate` call, the original JSON text is never modified--only read. After you are done
with the document, the source (whether file or string) can be safely discarded.

For best performance, a `parser` instance should be reused over several files: otherwise you will
needlessly reallocate memory, an expensive process. It is also possible to avoid entirely memory
allocations during parsing when using simdjson. [See our performance notes for details](performance.md).

Using the Parsed JSON
---------------------

Once you have a document, you can navigate it with idiomatic C++ iterators, operators and casts.
The following show how to use the JSON when exceptions are enabled, but simdjson has full, idiomatic
support for users who avoid exceptions. See [the simdjson error handling documentation](basics.md#error-handling) for more.

* **Validate What You Use:** When calling `iterate`, the document is quickly indexed. If it is
  not a valid UTF-8 string or if there is an unclosed string, an error may be reported right away.
  However, it is not fully validated. On Demand only fully validates the values you use and the
  structure leading to it.
* **Extracting Values:** You can cast a JSON element to a native type:
  `double(element)` or `double x = json_element`. This works for double, uint64_t, int64_t, bool,
  ondemand::object and ondemand::array. At this point, the number, string or boolean will be parsed,
  or the initial `[` or `{` will be verified. An exception is thrown if the cast is not possible.

  > IMPORTANT NOTE: values can only be parsed once. Since documents are *iterators*, once you have
  > parsed a value (such as by casting to double), you cannot get at it again.
* **Field Access:** To get the value of the "foo" field in an object, use `object["foo"]`. This will
  scan through the object looking for the field with the matching string, doing a character-by-character
  comparison.

  > NOTE: JSON allows you to escape characters in keys. E.g., the key `"date"` may be written as
  > `"\u0064\u0061\u0074\u0065"`. By default, simdjson does *not* unescape keys when matching by default.
  > Thus if you search for the key `"date"` and the JSON document uses `"\u0064\u0061\u0074\u0065"`
  > as a key, it will not be recognized. This is not generally a problem.  Nevertheless, if you do need
  > to support escaped keys, the method `unescaped_key()` provides the desired unescaped keys by
  > parsing and writing out the unescaped keys to a string buffer and returning a `std::string_view`
  > instance. You should expect a performance penalty when using `unescaped_key()`.
  > ```c++
  > auto json = R"({"k\u0065y": 1})"_padded;
  > ondemand::parser parser;
  > auto doc = parser.iterate(json);
  > ondemand::object object = doc.get_object();
  > for(auto field : object) {
  >    // parses and writes out the key, after unescaping it,
  >    // to a string buffer. It causes a performance penalty.
  >    std::string_view keyv = field.unescaped_key();
  >    if(keyv == "key") { std::cout << uint64_t(field.value()); }
  >  }
  > ```
  >
  > By default, field lookup is order-insensitive, so you can look up values in any order. However,
  > we still encourage you to look up fields in the order you expect them in the JSON, as it is still
  > faster.
  >
  > If you want to enforce finding fields in order, you can use `object.find_field("foo")` instead.
  > This will only look forward, and will fail to find fields in the wrong order: for example, this
  > will fail:
  >
  > ```c++
  > ondemand::parser parser;
  > auto json = R"(  { "x": 1, "y": 2 }  )"_padded;
  > auto doc = parser.iterate(json);
  > double y = doc.find_field("y"); // The cursor is now after the 2 (at })
  > double x = doc.find_field("x"); // This fails, because there are no more fields after "y"
  > ```
  >
  > By contrast, using the default (order-insensitive) lookup succeeds:
  >
  > ```c++
  > ondemand::parser parser;
  > auto json = R"(  { "x": 1, "y": 2 }  )"_padded;
  > auto doc = parser.iterate(json);
  > double y = doc["y"]; // The cursor is now after the 2 (at })
  > double x = doc["x"]; // Success: [] loops back around to find "x"
  > ```
* **Array Iteration:** To iterate through an array, use `for (auto value : array) { ... }`. This will
  step through each value in the JSON array.

  If you know the type of the value, you can cast it right there, too! `for (double value : array) { ... }`.
* **Object Iteration:** You can iterate through an object's fields, as well: `for (auto field : object) { ... }`
  - `field.unescaped_key()` will get you the unescaped key string.
  - `field.value()` will get you the value, which you can then use all these other methods on.
* **Array Index:** Because it is forward-only, you cannot look up an array element by index. Instead,
  you will need to iterate through the array and keep an index yourself.
* **Output to strings (simdjson 1.0 or better):** Given a document or an element (or node) out of a JSON document, you can output a JSON string version suitable to be parsed again as JSON content: `simdjson::to_string(element)` returns a `simdjson::simdjson_result<std::string>` instance. You can cast it to `std::string` and it will throw when an error was encountered (`std::string(simdjson::to_string(element))`). Or else you can do `std::string s; if(simdjson::to_string(element).get(s) == simdjson::SUCCESS) { ... }`. This consumes fully the element: if you apply it on a document, the JSON pointer is advanced to the end of the document. The returned string contains a serialized version of the element or document that is suitable to be parsed again. It is also a newly allocated `std::string` that is independent from the simdjson parser. The `to_string` function should not be confused with retrieving the value of a string instance which are escaped and represented using a lightweight `std::string_view` instance pointing at an internal string buffer inside the parser instance. To illustrate, the first of the following two code segments will print the unescaped string `"test"` complete with the quote whereas the second one will print the escaped content of the string (without the quotes). Th
  > ```C++
  > // serialize a JSON to an escaped std::string instance so that it can be parsed again as JSON
  > auto cars_json = R"( { "test": "result"  }  )"_padded;
  > ondemand::document doc = parser.iterate(cars_json);
  > std::cout << simdjson::to_string(doc["test"]) << std::endl; // Requires simdjson 1.0 or better
  >````
  > ```C++
  > // retrieves an unescaped string value as a string_view instance
  > auto cars_json = R"( { "test": "result"  }  )"_padded;
  > ondemand::document doc = parser.iterate(cars_json);
  > std::cout << std::string_view(doc["test"]) << std::endl;
  >````

### Examples

The following code illustrates many of the above concepts:

```c++
ondemand::parser parser;
auto cars_json = R"( [
  { "make": "Toyota", "model": "Camry",  "year": 2018, "tire_pressure": [ 40.1, 39.9, 37.7, 40.4 ] },
  { "make": "Kia",    "model": "Soul",   "year": 2012, "tire_pressure": [ 30.1, 31.0, 28.6, 28.7 ] },
  { "make": "Toyota", "model": "Tercel", "year": 1999, "tire_pressure": [ 29.8, 30.0, 30.2, 30.5 ] }
] )"_padded;

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
```

Here is a different example illustrating the same ideas:

```C++
ondemand::parser parser;
auto points_json = R"( [
    {  "12345" : {"x":12.34, "y":56.78, "z": 9998877}   },
    {  "12545" : {"x":11.44, "y":12.78, "z": 11111111}  }
  ] )"_padded;

// Parse and iterate through an array of objects
for (ondemand::object points : parser.iterate(points_json)) {
  for (auto point : points) {
    cout << "id: " << std::string_view(point.unescaped_key()) << ": (";
    cout << point.value()["x"].get_double() << ", ";
    cout << point.value()["y"].get_double() << ", ";
    cout << point.value()["z"].get_int64() << endl;
  }
}
```

And another one:

```C++
auto abstract_json = R"(
  { "str" : { "123" : {"abc" : 3.14 } } }
)"_padded;
ondemand::parser parser;
auto doc = parser.iterate(abstract_json);
cout << doc["str"]["123"]["abc"].get_double() << endl; // Prints 3.14
```

* **Extracting Values (without exceptions):** You can use a variant usage of `get()` with error
  codes to avoid exceptions. You first declare the variable of the appropriate type (`double`,
  `uint64_t`, `int64_t`, `bool`, `ondemand::object` and `ondemand::array`) and pass it by reference
  to `get()` which gives you back an error code: e.g.,

  ```c++
  auto abstract_json = R"(
    { "str" : { "123" : {"abc" : 3.14 } } }
  )"_padded;
  ondemand::parser parser;

  double value;
  auto doc = parser.iterate(abstract_json);
  auto error = doc["str"]["123"]["abc"].get(value);
  if (error) { std::cerr << error << std::endl; return EXIT_FAILURE; }
  cout << value << endl; // Prints 3.14
  ```

Sometimes it is useful to scan an array to determine its length prior to parsing it.
For this purpose, `array` instances have a `count_elements` method. Users should be
aware that the `count_elements` method can be costly since it requires scanning the
whole array. You may use it as follows if your document is itself an array:

```C++
  auto cars_json = R"( [ 40.1, 39.9, 37.7, 40.4 ] )"_padded;
  auto doc = parser.iterate(cars_json);
  size_t count = doc.count_elements(); // requires simdjson 1.0 or better
  std::vector<double> values(count);
  size_t index = 0;
  for(double x : doc) { values[index++] = x; }
```

If you access an array inside a document, you can use the `count_elements` method as follow.
You should not let the array instance go out of scope before consuming it after calling the `count_elements` method:
``` C++
   ondemand::parser parser;
   auto cars_json = R"( { "test":[ { "val1":1, "val2":2 }, { "val1":1, "val2":2 } ] }   )"_padded;
   auto doc = parser.iterate(cars_json);
   auto test_array = doc.find_field("test").get_array();
   size_t count = test_array.count_elements(); // requires simdjson 1.0 or better
   std::cout << "Number of elements: " <<  count << std::endl;
   for(ondemand::object elem: test_array) {
     std::cout << simdjson::to_string(elem);
   }
```

Tree Walking and JSON Element Types: Sometimes you don't necessarily have a document with a known type, and are trying to generically inspect or walk over JSON elements. To do that, you can use iterators and the type() method. For example, here's a quick and dirty recursive function that verbosely prints the JSON document as JSON:

```c++
// We use a template function because we need to
// support both ondemand::value and ondemand::document
// as a parameter type. Note that we move the values.
template <class T>
void recursive_print_json(T&& element) {
  bool add_comma;
  switch (element.type()) {
  case ondemand::json_type::array:
    cout << "[";
    add_comma = false;
    for (auto child : element.get_array()) {
      if (add_comma) {
        cout << ",";
      }
      // We need the call to value() to get
      // an ondemand::value type.
      recursive_print_json(child.value());
      add_comma = true;
    }
    cout << "]";
    break;
  case ondemand::json_type::object:
    cout << "{";
    add_comma = false;
    for (auto field : element.get_object()) {
      if (add_comma) {
        cout << ",";
      }
      // key() returns the key as it appears in the raw
      // JSON document, if we want the unescaped key,
      // we should do field.unescaped_key().
      cout << "\"" << field.key() << "\": ";
      recursive_print_json(field.value());
      add_comma = true;
    }
    cout << "}";
    break;
  case ondemand::json_type::number:
    // assume it fits in a double
    cout << element.get_double();
    break;
  case ondemand::json_type::string:
    // get_string() would return escaped string, but
    // we are happy with unescaped string.
    cout << "\"" << element.get_raw_json_string() << "\"";
    break;
  case ondemand::json_type::boolean:
    cout << element.get_bool();
    break;
  case ondemand::json_type::null:
    cout << "null";
    break;
  }
}

void basics_treewalk() {
  ondemand::parser parser;
  auto json = padded_string::load("twitter.json");
  recursive_print_json(parser.iterate(json));
}
```


C++11 Support and string_view
-------------

The simdjson library builds on compilers supporting the [C++11 standard](https://en.wikipedia.org/wiki/C%2B%2B11). It is also a strict requirement: we have no plan to support older C++ compilers.

We represent parsed strings in simdjson using the `std::string_view` class. It avoids
the need to copy the data, as would be necessary with the `std::string` class. It also
avoids the pitfalls of null-terminated C strings.

The `std::string_view` class has become standard as part of C++17 but it is not always available
on compilers which only supports C++11. When we detect that `string_view` is natively
available, we define the macro `SIMDJSON_HAS_STRING_VIEW`.

When we detect that it is unavailable,
we use [string-view-lite](https://github.com/martinmoene/string-view-lite) as a
substitute. In such cases, we use the type alias `using string_view = nonstd::string_view;` to
offer the same API, irrespective of the compiler and standard library. The macro
`SIMDJSON_HAS_STRING_VIEW` will be *undefined* to indicate that we emulate `string_view`.


C++17 Support
-------------

While the simdjson library can be used in any project using C++ 11 and above, field iteration has special support C++ 17's destructuring syntax. For example:

```c++
padded_string json = R"(  { "foo": 1, "bar": 2 }  )"_padded;
dom::parser parser;
dom::object object;
auto error = parser.parse(json).get(object);
if (error) { cerr << error << endl; return; }
for (auto [key, value] : object) {
  cout << key << " = " << value << endl;
}
```

For comparison, here is the C++ 11 version of the same code:

```c++
// C++ 11 version for comparison
padded_string json = R"(  { "foo": 1, "bar": 2 }  )"_padded;
dom::parser parser;
dom::object object;
auto error = parser.parse(json).get(object);
if (error) { cerr << error << endl; return; }
for (dom::key_value_pair field : object) {
  cout << field.key << " = " << field.value << endl;
}
```

Minifying JSON strings without parsing
----------------------

In some cases, you may have valid JSON strings that you do not wish to parse but that you wish to minify. That is, you wish to remove all unnecessary spaces. We have a fast function for this purpose (`simdjson::minify(const char * input, size_t length, const char * output, size_t& new_length)`). This function does not validate your content, and it does not parse it.  It is much faster than parsing the string and re-serializing it in minified form (`simdjson::minify(parser.parse())`). Usage is relatively simple. You must pass an input pointer with a length parameter, as well as an output pointer and an output length parameter (by reference). The output length parameter is not read, but written to. The output pointer should point to a valid memory region that is as large as the original string length. The input pointer and input length are read, but not written to.

```C++
  // Starts with a valid JSON document as a string.
  // It does not have to be null-terminated.
  const char * some_string = "[ 1, 2, 3, 4] ";
  size_t length = std::strlen(some_string);
  // Create a buffer to receive the minified string. Make sure that there is enough room (length bytes).
  std::unique_ptr<char[]> buffer{new char[length]};
  size_t new_length{}; // It will receive the minified length.
  auto error = simdjson::minify(some_string, length, buffer.get(), new_length);
  // The buffer variable now has "[1,2,3,4]" and new_length has value 9.
```

Though it does not validate the JSON input, it will detect when the document ends with an unterminated string. E.g., it would refuse to minify the string `"this string is not terminated` because of the missing final quote.


UTF-8 validation (alone)
----------------------

The simdjson library has fast functions to validate UTF-8 strings. They are many times faster than most functions commonly found in libraries. You can use our fast functions, even if you do not care about JSON.

```C++
  const char * some_string = "[ 1, 2, 3, 4] ";
  size_t length = std::strlen(some_string);
  bool is_ok = simdjson::validate_utf8(some_string, length);
```

The UTF-8 validation function merely checks that the input is valid UTF-8: it works with strings in general, not just JSON strings.

Your input string does not need any padding. Any string will do. The `validate_utf8` function does not do any memory allocation on the heap, and it does not throw exceptions.

JSON Pointer
------------

The simdjson library also supports [JSON pointer](https://tools.ietf.org/html/rfc6901) through the
`at_pointer()` method, letting you reach further down into the document in a single call:

```c++
auto cars_json = R"( [
  { "make": "Toyota", "model": "Camry",  "year": 2018, "tire_pressure": [ 40.1, 39.9, 37.7, 40.4 ] },
  { "make": "Kia",    "model": "Soul",   "year": 2012, "tire_pressure": [ 30.1, 31.0, 28.6, 28.7 ] },
  { "make": "Toyota", "model": "Tercel", "year": 1999, "tire_pressure": [ 29.8, 30.0, 30.2, 30.5 ] }
] )"_padded;
dom::parser parser;
dom::element cars = parser.parse(cars_json);
cout << cars.at_pointer("/0/tire_pressure/1") << endl; // Prints 39.9
```

A JSON Path is a sequence of segments each starting with the '/' character. Within arrays, an integer
index allows you to select the indexed node. Within objects, the string value of the key allows you to
select the value. If your keys contain the characters '/' or '~', they must be escaped as '~1' and
'~0' respectively. An empty JSON Path refers to the whole document.

We also extend the JSON Pointer support to include *relative* paths.
You can apply a JSON path to any node and the path gets interpreted relatively, as if the current node were a whole JSON document.

Consider the following example:

```c++
auto cars_json = R"( [
  { "make": "Toyota", "model": "Camry",  "year": 2018, "tire_pressure": [ 40.1, 39.9, 37.7, 40.4 ] },
  { "make": "Kia",    "model": "Soul",   "year": 2012, "tire_pressure": [ 30.1, 31.0, 28.6, 28.7 ] },
  { "make": "Toyota", "model": "Tercel", "year": 1999, "tire_pressure": [ 29.8, 30.0, 30.2, 30.5 ] }
] )"_padded;
dom::parser parser;
dom::element cars = parser.parse(cars_json);
cout << cars.at_pointer("/0/tire_pressure/1") << endl; // Prints 39.9
for (dom::element car_element : cars) {
    dom::object car;
    simdjson::error_code error;
    if ((error = car_element.get(car))) { std::cerr << error << std::endl; return; }
    double x = car.at_pointer("/tire_pressure/1");
    cout << x << endl; // Prints 39.9, 31 and 30
}
```



Error Handling
--------------

All simdjson APIs that can fail return `simdjson_result<T>`, which is a &lt;value, error_code&gt;
pair. You can retrieve the value with .get(), like so:

```c++
dom::element doc;
auto error = parser.parse(json).get(doc);
if (error) { cerr << error << endl; exit(1); }
```

When you use the code this way, it is your responsibility to check for error before using the
result: if there is an error, the result value will not be valid and using it will caused undefined
behavior.

We can write a "quick start" example where we attempt to parse the following JSON file and access some data, without triggering exceptions:
```JavaScript
{
  "statuses": [
    {
      "id": 505874924095815700
    },
    {
      "id": 505874922023837700
    }
  ],
  "search_metadata": {
    "count": 100
  }
}
```

Our program loads the file, selects value corresponding to key "search_metadata" which expected to be an object, and then
it selects the key "count" within that object.

```C++
#include "simdjson.h"

int main(void) {
  simdjson::dom::parser parser;
  simdjson::dom::element tweets;
  auto error = parser.load("twitter.json").get(tweets);
  if (error) { std::cerr << error << std::endl; return EXIT_FAILURE; }

  simdjson::dom::element res;
  if ((error = tweets["search_metadata"]["count"].get(res))) {
    std::cerr << "could not access keys" << std::endl;
    return EXIT_FAILURE;
  }
  std::cout << res << " results." << std::endl;
}
```

The following is a similar example where one wants to get the id of the first tweet without
triggering exceptions. To do this, we use `["statuses"].at(0)["id"]`. We break that expression down:

- Get the list of tweets (the `"statuses"` key of the document) using `["statuses"]`). The result is expected to be an array.
- Get the first tweet using `.at(0)`. The result is expected to be an object.
- Get the id of the tweet using ["id"]. We expect the value to be a non-negative integer.

Observe how we use the `at` method when querying an index into an array, and not the bracket operator.

```C++
#include "simdjson.h"

int main(void) {
  simdjson::dom::parser parser;
  simdjson::dom::element tweets;
  auto error = parser.load("twitter.json").get(tweets);
  if(error) { std::cerr << error << std::endl; return EXIT_FAILURE; }
  uint64_t identifier;
  error = tweets["statuses"].at(0)["id"].get(identifier);
  if(error) { std::cerr << error << std::endl; return EXIT_FAILURE; }
  std::cout << identifier << std::endl;
  return EXIT_SUCCESS;
}
```

### Error Handling Example

This is how the example in "Using the Parsed JSON" could be written using only error code checking:

```c++
auto cars_json = R"( [
  { "make": "Toyota", "model": "Camry",  "year": 2018, "tire_pressure": [ 40.1, 39.9, 37.7, 40.4 ] },
  { "make": "Kia",    "model": "Soul",   "year": 2012, "tire_pressure": [ 30.1, 31.0, 28.6, 28.7 ] },
  { "make": "Toyota", "model": "Tercel", "year": 1999, "tire_pressure": [ 29.8, 30.0, 30.2, 30.5 ] }
] )"_padded;
dom::parser parser;
dom::array cars;
auto error = parser.parse(cars_json).get(cars);
if (error) { cerr << error << endl; exit(1); }

// Iterating through an array of objects
for (dom::element car_element : cars) {
    dom::object car;
    if ((error = car_element.get(car))) { cerr << error << endl; exit(1); }

    // Accessing a field by name
    std::string_view make, model;
    if ((error = car["make"].get(make))) { cerr << error << endl; exit(1); }
    if ((error = car["model"].get(model))) { cerr << error << endl; exit(1); }
    cout << "Make/Model: " << make << "/" << model << endl;

    // Casting a JSON element to an integer
    uint64_t year;
    if ((error = car["year"].get(year))) { cerr << error << endl; exit(1); }
    cout << "- This car is " << 2020 - year << "years old." << endl;

    // Iterating through an array of floats
    double total_tire_pressure = 0;
    dom::array tire_pressure_array;
    if ((error = car["tire_pressure"].get(tire_pressure_array))) { cerr << error << endl; exit(1); }
    for (dom::element tire_pressure_element : tire_pressure_array) {
        double tire_pressure;
        if ((error = tire_pressure_element.get(tire_pressure))) { cerr << error << endl; exit(1); }
        total_tire_pressure += tire_pressure;
    }
    cout << "- Average tire pressure: " << (total_tire_pressure / 4) << endl;

    // Writing out all the information about the car
    for (auto field : car) {
        cout << "- " << field.key << ": " << field.value << endl;
    }
}
```

Here is another example:

```C++
auto abstract_json = R"( [
    {  "12345" : {"a":12.34, "b":56.78, "c": 9998877}   },
    {  "12545" : {"a":11.44, "b":12.78, "c": 11111111}  }
  ] )"_padded;
dom::parser parser;
dom::array array;
auto error = parser.parse(abstract_json).get(array);
if (error) { cerr << error << endl; exit(1); }
// Iterate through an array of objects
for (dom::element elem : array) {
    dom::object obj;
    if ((error = elem.get(obj))) { cerr << error << endl; exit(1); }
    for (auto & key_value : obj) {
        cout << "key: " << key_value.key << " : ";
        dom::object innerobj;
        if ((error = key_value.value.get(innerobj))) { cerr << error << endl; exit(1); }

        double va, vb;
        if ((error = innerobj["a"].get(va))) { cerr << error << endl; exit(1); }
        cout << "a: " << va << ", ";
        if ((error = innerobj["b"].get(vc))) { cerr << error << endl; exit(1); }
        cout << "b: " << vb << ", ";

        int64_t vc;
        if ((error = innerobj["c"].get(vc))) { cerr << error << endl; exit(1); }
        cout << "c: " << vc << endl;
    }
}
```

And another one:

```C++
  auto abstract_json = R"(
    {  "str" : { "123" : {"abc" : 3.14 } } } )"_padded;
  dom::parser parser;
  double v;
  auto error = parser.parse(abstract_json)["str"]["123"]["abc"].get(v);
  if (error) { cerr << error << endl; exit(1); }
  cout << "number: " << v << endl;
```

Notice how we can string several operations (`parser.parse(abstract_json)["str"]["123"]["abc"].get(v)`) and only check for the error once, a strategy we call  *error chaining*.

The next two functions will take as input a JSON document containing an array with a single element, either a string or a number. They return true upon success.

```C++
simdjson::dom::parser parser{};

bool parse_double(const char *j, double &d) {
  auto error = parser.parse(j, std::strlen(j))
        .at(0)
        .get(d, error);
  if (error) { return false; }
  return true;
}

bool parse_string(const char *j, std::string &s) {
  std::string_view answer;
  auto error = parser.parse(j,strlen(j))
        .at(0)
        .get(answer, error);
  if (error) { return false; }
  s.assign(answer.data(), answer.size());
  return true;
}
```

To ensure you don't write any code that uses exceptions, compile with `SIMDJSON_EXCEPTIONS=OFF`. For example, if including the project via cmake:

```cmake
target_compile_definitions(simdjson PUBLIC SIMDJSON_EXCEPTIONS=OFF)
```

### Exceptions

Users more comfortable with an exception flow may choose to directly cast the `simdjson_result<T>` to the desired type:

```c++
dom::element doc = parser.parse(json); // Throws an exception if there was an error!
```

When used this way, a `simdjson_error` exception will be thrown if an error occurs, preventing the
program from continuing if there was an error.


If one is willing to trigger exceptions, it is possible to write simpler code:

```C++
#include "simdjson.h"

int main(void) {
  simdjson::dom::parser parser;
  simdjson::dom::element tweets = parser.load("twitter.json");
  std::cout << "ID: " << tweets["statuses"].at(0)["id"] << std::endl;
  return EXIT_SUCCESS;
}
```


Rewinding
----------

In some instances, you may need to go through a document more than once. For that purpose, you may
call the `rewind()` method on the document instance. It allows you to restart processing from the
beginning without rescanning all of the input data again. It invalidates all values, objects and arrays
that you have created so far (including unescaped strings).

In the following example, we print on the screen the number of cars in the JSON input file
before printout the data.

```C++
  ondemand::parser parser;
  auto cars_json = R"( [
    { "make": "Toyota", "model": "Camry",  "year": 2018, "tire_pressure": [ 40.1, 39.9, 37.7, 40.4 ] },
    { "make": "Kia",    "model": "Soul",   "year": 2012, "tire_pressure": [ 30.1, 31.0, 28.6, 28.7 ] },
    { "make": "Toyota", "model": "Tercel", "year": 1999, "tire_pressure": [ 29.8, 30.0, 30.2, 30.5 ] }
  ] )"_padded;

  auto doc = parser.iterate(cars_json);
  for (simdjson_unused ondemand::object car : doc) {
    if(car["make"] == "Toyota") { count++; }
  }
  std::cout << "We have " << count << " Toyota cars.\n";
  doc.rewind(); // requires simdjson 1.0 or better
  for (ondemand::object car : doc) {
    cout << "Make/Model: " << std::string_view(car["make"]) << "/" << std::string_view(car["model"]) << endl;
  }
```


Direct Access to the Raw String
--------------------------------

The simdjson library makes explicit assumptions about types. For examples, numbers
must be integers (up to 64-bit integers) or binary64 floating-point numbers. Some users
have different needs. For example, some users might want to support big integers.
The library makes this possible by providing a `raw_json_token` method which returns
a `std::string_view` instance containing the value as a string which you may then
parse as you see fit.

```C++
simdjson::ondemand::parser parser;
simdjson::padded_string docdata =  R"({"value":12321323213213213213213213213211223})"_padded;
simdjson::ondemand::document doc = parser.iterate(docdata);
simdjson::ondemand::object obj = doc.get_object();
std::string_view token = obj["value"].raw_json_token();
// token has value "12321323213213213213213213213211223"
```

Performance note: the On Demand front-end does not materialize the parsed numbers and other values. If you are accessing everything twice, you may need to parse them twice. Thus the rewind functionality is
best suited for cases where the first pass only scans the structure of the document.
The `raw_json_token` method even works when the JSON value is a string. In such cases, it
will return the complete string with the quotes and with eventual escaped sequences as in the
source document.

Newline-Delimited JSON (ndjson) and JSON lines
----------------------------------------------

The simdjson library also support multithreaded JSON streaming through a large file containing many
smaller JSON documents in either [ndjson](http://ndjson.org) or [JSON lines](http://jsonlines.org)
format. If your JSON documents all contain arrays or objects, we even support direct file
concatenation without whitespace. The concatenated file has no size restrictions (including larger
than 4GB), though each individual document must be no larger than 4 GB.

Here is a simple example, given `x.json` with this content:

```json
{ "foo": 1 }
{ "foo": 2 }
{ "foo": 3 }
```

```c++
dom::parser parser;
dom::document_stream docs = parser.load_many("x.json");
for (dom::element doc : docs) {
  cout << doc["foo"] << endl;
}
// Prints 1 2 3
```

In-memory ndjson strings can be parsed as well, with `parser.parse_many(string)`:


```c++
dom::parser parser;
  auto json = R"({ "foo": 1 }
{ "foo": 2 }
{ "foo": 3 })"_padded;
dom::document_stream docs = parser.parse_many(json);
for (dom::element doc : docs) {
  cout << doc["foo"] << endl;
}
// Prints 1 2 3
```


Unlike `parser.parse`, both `parser.load_many(filename)` and `parser.parse_many(string)` may parse
"on demand" (lazily). That is, no parsing may have been done before you enter the loop
`for (dom::element doc : docs) {` and you should expect the parser to only ever fully parse one JSON
document at a time.

1. When calling `parser.load_many(filename)`, the file's content is loaded up in a memory buffer owned by the `parser`'s instance. Thus the file can be safely deleted after calling `parser.load_many(filename)` as the parser instance owns all of the data.
2. When calling  `parser.parse_many(string)`, no copy is made of the provided string input. The provided memory buffer may be accessed each time a JSON document is parsed.  Calling `parser.parse_many(string)` on a  temporary string buffer (e.g., `docs = parser.parse_many("[1,2,3]"_padded)`) is unsafe (and will not compile) because the  `document_stream` instance needs access to the buffer to return the JSON documents. In contrast, calling `doc = parser.parse("[1,2,3]"_padded)` is safe because `parser.parse` eagerly parses the input.


Both `load_many` and `parse_many` take an optional parameter `size_t batch_size` which defines the window processing size. It is set by default to a large value (`1000000` corresponding to 1 MB). None of your JSON documents should exceed this window size, or else you will get  the error `simdjson::CAPACITY`. You cannot set this window size larger than 4 GB: you will get  the error `simdjson::CAPACITY`. The smaller the window size is, the less memory the function will use. Setting the window size too small (e.g., less than 100 kB) may also impact performance negatively. Leaving it to 1 MB is expected to be a good choice, unless you have some larger documents.

If your documents are large (e.g., larger than a megabyte), then the `load_many` and `parse_many` functions are maybe ill-suited. They are really meant to support reading efficiently streams of relatively small documents (e.g., a few kilobytes each). If you have larger documents, you should use other functions like `parse`.

See [parse_many.md](parse_many.md) for detailed information and design.

Thread Safety
-------------

We built simdjson with thread safety in mind.

The simdjson library is single-threaded except for  [`parse_many`](parse_many.md) which may use secondary threads under its control when the library is compiled with thread support.


We recommend using one `dom::parser` object per thread in which case the library is thread-safe.
It is unsafe to reuse a `dom::parser` object between different threads.
The parsed results (`dom::document`, `dom::element`, `array`, `object`) depend on the `dom::parser`, etc. therefore it is also potentially unsafe to use the result of the parsing between different threads.

The CPU detection, which runs the first time parsing is attempted and switches to the fastest
parser for your CPU, is transparent and thread-safe.


Standard Compliance
--------------------

The simdjson library is fully compliant with  the [RFC 8259](https://www.tbray.org/ongoing/When/201x/2017/12/14/rfc8259.html) JSON specification.

- The only insignificant whitespace characters allowed are the space, the horizontal tab, the line feed and the carriage return. In particular, a JSON document may not contain an unespaced null character.
- A single string or a single number is considered to be a valid JSON document.
- We fully validate the numbers according to the JSON specification. For example,  the string `01` is not valid JSON document since the specification states that *leading zeros are not allowed*.
- The specification allows implementations to set limits on the range and precision of numbers accepted.  We support 64-bit floating-point numbers as well as integer values.
  - We parse integers and floating-point numbers as separate types which allows us to support all signed (two's complement) 64-bit integers, like a Java `long` or a C/C++ `long long` and all 64-bit unsigned integers. When we cannot represent exactly an integer as a signed or unsigned 64-bit value, we reject the JSON document.
  - We support the full range of 64-bit floating-point numbers (binary64). The values range from `std::numeric_limits<double>::lowest()`  to `std::numeric_limits<double>::max()`, so from -1.7976e308 all the way to 1.7975e308. Extreme values (less or equal to -1e308, greater or equal to 1e308) are rejected: we refuse to parse the input document. Numbers are parsed with a perfect accuracy (ULP 0): the nearest floating-point value is chosen, rounding to even when needed. If you serialized your floating-point numbers with 17 significant digits in a standard compliant manner, the simdjson library is guaranteed to recover the same numbers, exactly.
- The specification states that JSON text exchanged between systems that are not part of a closed ecosystem MUST be encoded using UTF-8. The simdjson library does full UTF-8 validation as part of the parsing. The specification states that implementations MUST NOT add a byte order mark: the simdjson library rejects documents starting with a  byte order mark.
- The simdjson library validates string content for unescaped characters. Unescaped line breaks and tabs in strings are not allowed.
- The simdjson library accepts objects with repeated keys: all of the name/value pairs, including duplicates, are reported. We do not enforce key uniqueness.
- The specification states that an implementation may set limits on the size of texts that it accepts. The simdjson library limits single JSON documents to 4 GiB. It will refuse to parse a JSON document larger than 4294967295 bytes. (This limitation does not apply to streams of JSON documents, only to single JSON documents.)
- The specification states that an implementation may set limits on the maximum depth of nesting. By default, the simdjson will refuse to parse documents with a depth exceeding 1024.


Backwards Compatibility
-----------------------

The only header file supported by simdjson is `simdjson.h`. Older versions of simdjson published a
number of other include files such as `document.h` or `ParsedJson.h` alongside `simdjson.h`; these headers
may be moved or removed in future versions.
