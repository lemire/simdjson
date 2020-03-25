[![Build Status](https://cloud.drone.io/api/badges/simdjson/simdjson/status.svg)](https://cloud.drone.io/simdjson/simdjson)
[![CircleCI](https://circleci.com/gh/simdjson/simdjson.svg?style=svg)](https://circleci.com/gh/simdjson/simdjson)
[![Fuzzing Status](https://oss-fuzz-build-logs.storage.googleapis.com/badges/simdjson.svg)](https://bugs.chromium.org/p/oss-fuzz/issues/list?sort=-opened&q=proj%3Asimdjson&can=2)
[![Build status](https://ci.appveyor.com/api/projects/status/ae77wp5v3lebmu6n/branch/master?svg=true)](https://ci.appveyor.com/project/lemire/simdjson-jmmti/branch/master)
[![][license img]][license]

simdjson : Parsing gigabytes of JSON per second
===============================================

<img src="images/logo.png" width="10%" style="float: right">
JSON is everywhere on the Internet. Servers spend a *lot* of time parsing it. We need a fresh
approach. The simdjson library uses commonly available SIMD instructions and microparallel algorithms
to parse JSON 2.5x faster than anything else out there.

* **Fast:** Over 2.5x faster than other production-grade JSON parsers.
* **Easy:** First-class, easy to use API.
* **Strict:** Full JSON and UTF-8 validation, lossless parsing. Performance with no compromises.
* **Automatic:** Selects a CPU-tailored parser at runtime. No configuration needed.
* **Reliable:** From memory allocation to error handling, simdjson's design avoids surprises.

This library is part of the [Awesome Modern C++](https://awesomecpp.com) list.

Table of Contents
-----------------

* [Quick Start](#quick-start)
* [Documentation](#documentation)
* [Performance results](#performance-results)
* [Real-world usage](#real-world-usage)
* [Bindings and Ports of simdjson](#bindings-and-ports-of-simdjson)
* [About simdjson](#about-simdjson)
* [Funding](#funding)
* [Contributing to simdjson](#contributing-to-simdjson)
* [License](#license)

Quick Start
-----------

The simdjson library is easily consumable with a single .h and .cpp file.

0. Prerequisites: `g++` or `clang++`.
1. Pull [simdjson.h](singleheader/simdjson.h) and [simdjson.cpp](singleheader/simdjson.cpp) into a directory, along with the sample file [twitter.json](jsonexamples/twitter.json).
   ```
   wget https://raw.githubusercontent.com/simdjson/simdjson/master/singleheader/simdjson.h https://raw.githubusercontent.com/simdjson/simdjson/master/singleheader/simdjson.cpp https://raw.githubusercontent.com/simdjson/simdjson/master/jsonexamples/twitter.json
   ```
2. Create `parser.cpp`:

   ```c++
   #include "simdjson.h"
   int main(void) {
     simdjson::document::parser parser;
     simdjson::document& tweets = parser.load("twitter.json");
     std::cout << tweets["search_metadata"]["count"] << " results." << std::endl;
   }
   ```
3. `c++ -o parser parser.cpp simdjson.cpp -std=c++17`
4. `./parser`
   ```
   100 results.
   ```

Documentation
-------------

Usage documentation is available:

* [Basics](doc/basics.md) is an overview of how to use simdjson and its APIs.
* [Performance](doc/performance.md) shows some more advanced scenarios and how to tune for them.
* [Implementation Selection](doc/implementation-selection.md) describes runtime CPU detection and
  how you can work with it.

Performance results
-------------------

The simdjson library uses three-quarters less instructions than state-of-the-art parser RapidJSON and
fifty percent less than sajson. To our knowledge, simdjson is the first fully-validating JSON parser
to run at gigabytes per second on commodity processors.

<img src="doc/gbps.png" width="90%">

On a Skylake processor, the parsing speeds (in GB/s) of various processors on the twitter.json file
are as follows.

| parser                                | GB/s |
| ------------------------------------- | ---- |
| simdjson                              | 2.2  |
| RapidJSON encoding-validation         | 0.51 |
| RapidJSON encoding-validation, insitu | 0.71 |
| sajson (insitu, dynamic)              | 0.70 |
| sajson (insitu, static)               | 0.97 |
| dropbox                               | 0.14 |
| fastjson                              | 0.26 |
| gason                                 | 0.85 |
| ultrajson                             | 0.42 |
| jsmn                                  | 0.28 |
| cJSON                                 | 0.34 |
| JSON for Modern C++ (nlohmann/json)   | 0.10 |

Real-world usage
----------------

- [Microsoft FishStore](https://github.com/microsoft/FishStore)
- [Yandex ClickHouse](https://github.com/yandex/ClickHouse)
- [Clang Build Analyzer](https://github.com/aras-p/ClangBuildAnalyzer)

If you are planning to use simdjson in a product, please work from one of our releases.

Bindings and Ports of simdjson
------------------------------

We distinguish between "bindings" (which just wrap the C++ code) and a port to another programming language (which reimplements everything).

- [ZippyJSON](https://github.com/michaeleisel/zippyjson): Swift bindings for the simdjson project.
- [pysimdjson](https://github.com/TkTech/pysimdjson): Python bindings for the simdjson project.
- [simdjson-rs](https://github.com/Licenser/simdjson-rs): Rust port.
- [simdjson-rust](https://github.com/SunDoge/simdjson-rust): Rust wrapper (bindings).
- [SimdJsonSharp](https://github.com/EgorBo/SimdJsonSharp): C# version for .NET Core (bindings and full port).
- [simdjson_nodejs](https://github.com/luizperes/simdjson_nodejs): Node.js bindings for the simdjson project.
- [simdjson_php](https://github.com/crazyxman/simdjson_php): PHP bindings for the simdjson project.
- [simdjson_ruby](https://github.com/saka1/simdjson_ruby): Ruby bindings for the simdjson project.
- [simdjson-go](https://github.com/minio/simdjson-go): Go port using Golang assembly.
- [rcppsimdjson](https://github.com/eddelbuettel/rcppsimdjson): R bindings.

About simdjson
--------------

The simdjson library takes advantage of modern microarchitectures, parallelizing with SIMD vector
instructions, reducing branch misprediction, and reducing data dependency to take advantage of each
CPU's multiple execution cores.

Some people [enjoy reading our paper](https://arxiv.org/abs/1902.08318): A description of the design
and implementation of simdjson is in our research article in VLDB journal: Geoff Langdale, Daniel
Lemire, [Parsing Gigabytes of JSON per Second](https://arxiv.org/abs/1902.08318), VLDB Journal 28 (6), 2019appear)

We also have an informal [blog post providing some background and context](https://branchfree.org/2019/02/25/paper-parsing-gigabytes-of-json-per-second/).

For the video inclined, [![simdjson at QCon San Francisco 2019](http://img.youtube.com/vi/wlvKAT7SZIQ/0.jpg)](http://www.youtube.com/watch?v=wlvKAT7SZIQ)
(it was the best voted talk, we're kinda proud of it).

Funding
-------

The work is supported by the Natural Sciences and Engineering Research Council of Canada under grant number RGPIN-2017-03910.

[license]: LICENSE
[license img]: https://img.shields.io/badge/License-Apache%202-blue.svg

Contributing to simdjson
------------------------

Head over to [CONTRIBUTING.md](CONTRIBUTING.md) for information on contributing to simdjson, and
[HACKING.md](HACKING.md) for information on source, building, and architecture/design.

License
-------

This code is made available under the Apache License 2.0.

Under Windows, we build some tools using the windows/dirent_portable.h file (which is outside our library code): it under the liberal (business-friendly) MIT license.
