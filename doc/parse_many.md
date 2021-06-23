parse_many
==========

An interface providing features to work with files or streams containing multiple small JSON documents.
As fast and convenient as possible.

Contents
--------

- [Motivations](#motivations)
- [Performance](#performance)
- [How it works](#how-it-works)
- [Support](#support)
- [API](#api)
- [Use cases](#use-cases)
- [Tracking your position](#tracking-your-position)
- [Incomplete streams](#incomplete-streams)

Motivation
-----------

The main motivation for this piece of software is to achieve maximum speed and offer a
better quality of life in parsing files containing multiple small JSON documents.

The JavaScript Object Notation (JSON) [RFC7159](https://tools.ietf.org/html/rfc7159) is a handy
serialization format.  However, when serializing a large sequence of
values as an array, or a possibly indeterminate-length or never-
ending sequence of values, JSON may be inconvenient.

Consider a sequence of one million values, each possibly one kilobyte
when encoded -- roughly one gigabyte.  It is often desirable to process such a dataset incrementally
without having to first read all of it before beginning to produce results.

Performance
-----------

The following is a chart comparing the speed of the different alternatives to parse a multiline JSON.
The simdjson library provides a threaded and non-threaded `parse_many()` implementation.  As the
figure below shows, if you can, use threads, but if you cannot, the unthreaded mode is still fast!
[![Chart.png](/doc/Multiline_JSON_Parse_Competition.png)](/doc/Multiline_JSON_Parse_Competition.png)

How it works
------------

### Context

The parsing in simdjson is divided into 2 stages.  First, in stage 1, we parse the document and find
all the structural indexes (`{`, `}`, `]`, `[`, `,`, `"`, ...) and validate UTF8.  Then, in stage 2,
we go through the document again and build the tape using structural indexes found during stage 1.
Although stage 1 finds the structural indexes, it has no knowledge of the structure of the document
nor does it know whether it parsed a valid document, multiple documents, or even if the document is
complete.

Prior to parse_many, most people who had to parse a multiline JSON file would proceed by reading the
file line by line, using a utility function like `std::getline` or equivalent, and would then use
the `parse` on each of those lines.  From a performance point of view, this process is highly
inefficient,  in that it requires a lot of unnecessary memory allocation and makes use of the
`getline` function, which is fundamentally slow, slower than the act of parsing with simdjson
[(more on this here)](https://lemire.me/blog/2019/06/18/how-fast-is-getline-in-c/).

Unlike the popular parser RapidJson, our DOM does not require the buffer once the parsing job is
completed,  the DOM and the buffer are completely independent. The drawback of this architecture is
that we need to allocate some additional memory to store our ParsedJson data, for every document
inside a given file.  Memory allocation can be slow and become a bottleneck, therefore, we want to
minimize it as much as possible.

### Design

To achieve a minimum amount of allocations, we opted for a design where we create only one
parser object and therefore allocate its memory once, and then recycle it for every document in a
given file. But, knowing that they often have largely varying size, we need to make sure that we
allocate enough memory so that all the documents can fit. This value is what we call the batch size.
As of right now, we need to manually specify a value for this batch size, it has to be at least as
big as the biggest document in your file, but not too big so that it submerges the cached memory.
The bigger the batch size, the fewer we need to make allocations. We found that 1MB is somewhat a
sweet spot.

1. When the user calls `parse_many`, we return a `document_stream` which the user can iterate over
   to receive parsed documents.
2. We call stage 1 on the first batch_size bytes of JSON in the buffer, detecting structural
   indexes for all documents in that batch.
3. We call stage 2 on the indexes, reading tokens until we reach the end of a valid document (i.e.
   a single array, object, string, boolean, number or null).
4. Each time the user calls `++` to read the next document, we call stage 2 to parse the next
   document where we left off.
5. When we reach the end of the batch, we call stage 1 on the next batch, starting from the end of
   the last document, and go to step 3.

### Threads

But how can we make use of threads if they are available?  We found a pretty cool algorithm that allows us to quickly
identify the  position of the last JSON document in a given batch. Knowing exactly where the end of
the batch is, we no longer need for stage 2 to finish in order to load a new batch. We already know
where to start the next batch. Therefore, we can run stage 1 on the next batch concurrently while
the main thread is going through stage 2. Running stage 1 in a different thread can, in best
cases, remove almost entirely its cost and replaces it by the overhead of a thread, which is orders
of magnitude cheaper. Ain't that awesome!

Thread support is only active if thread supported is detected in which case the macro
SIMDJSON_THREADS_ENABLED is set. Otherwise the library runs in  single-thread mode.

A `document_stream` instance uses at most two threads: there is a main thread and a worker thread.
You should expect the main thread to be fully occupied while the worker thread is partially busy
(e.g., 80% of the time).

Support
-------

Since we want to offer flexibility and not restrict ourselves to a specific file
format, we support any file that contains any amount of valid JSON document, **separated by one
or more character that is considered whitespace** by the JSON spec. Anything that is
 not whitespace will be parsed as a JSON document and could lead to failure.

Whitespace Characters:
- **Space**
- **Linefeed**
- **Carriage return**
- **Horizontal tab**
- **Nothing**

Some official formats **(non-exhaustive list)**:
- [Newline-Delimited JSON (NDJSON)](http://ndjson.org/)
- [JSON lines (JSONL)](http://jsonlines.org/)
- [Record separator-delimited JSON (RFC 7464)](https://tools.ietf.org/html/rfc7464) <- Not supported by JsonStream!
- [More on Wikipedia...](https://en.wikipedia.org/wiki/JSON_streaming)

API
---

See [basics.md](basics.md#newline-delimited-json-ndjson-and-json-lines) for an overview of the API.

## Use cases

From [jsonlines.org](http://jsonlines.org/examples/):

- **Better than CSV**
    ```json
    ["Name", "Session", "Score", "Completed"]
    ["Gilbert", "2013", 24, true]
    ["Alexa", "2013", 29, true]
    ["May", "2012B", 14, false]
    ["Deloise", "2012A", 19, true]
    ```
    CSV seems so easy that many programmers have written code to generate it themselves, and almost every implementation is
    different. Handling broken CSV files is a common and frustrating task. CSV has no standard encoding, no standard column
    separator and multiple character escaping standards. String is the only type supported for cell values, so some programs
     attempt to guess the correct types.

    JSON Lines handles tabular data cleanly and without ambiguity. Cells may use the standard JSON types.

    The biggest missing piece is an import/export filter for popular spreadsheet programs so that non-programmers can use
    this format.

- **Easy Nested Data**
    ```json
    {"name": "Gilbert", "wins": [["straight", "7♣"], ["one pair", "10♥"]]}
    {"name": "Alexa", "wins": [["two pair", "4♠"], ["two pair", "9♠"]]}
    {"name": "May", "wins": []}
    {"name": "Deloise", "wins": [["three of a kind", "5♣"]]}
    ```
    JSON Lines' biggest strength is in handling lots of similar nested data structures. One .jsonl file is easier to
    work with than a directory full of XML files.


Tracking your position
-----------

Some users would like to know where the document they parsed is in the input array of bytes.
It is possible to do so by accessing directly the iterator and calling its `current_index()`
method which reports the location (in bytes) of the current document in the input stream.
You may also call the `source()` method to get a `std::string_view` instance on the document.

Let us illustrate the idea with code:


```C++
    auto json = R"([1,2,3]  {"1":1,"2":3,"4":4} [1,2,3]  )"_padded;
    simdjson::dom::parser parser;
    simdjson::dom::document_stream stream;
    auto error = parser.parse_many(json).get(stream);
    if( error ) { /* do something */ }
    auto i = stream.begin();
    for(; i != stream.end(); ++i) {
        auto doc = *i;
        if(!doc.error()) {
          std::cout << "got full document at " << i.current_index() << std::endl;
          std::cout << i.source() << std::endl;
          count++;
        } else {
          std::cout << "got broken document at " << i.current_index() << std::endl;
          return false;
        }
    }

```

This code will print:
```
got full document at 0
[1,2,3]
got full document at 9
{"1":1,"2":3,"4":4}
got full document at 29
[1,2,3]
```


Incomplete streams
-----------

Some users may need to work with truncated streams. The simdjson may truncate documents at the very end of the stream that cannot possibly be valid JSON (e.g., they contain unclosed strings, unmatched brackets, unmatched braces). After iterating through the stream, you may query the `truncated_bytes()` method which tells you how many bytes were truncated. If the stream is made of full (whole) documents, then you should expect `truncated_bytes()` to return zero.


Consider the following example where a truncated document (`{"key":"intentionally unclosed string  `) containing 39 bytes has been left within the stream. In such cases, the first two whole documents are parsed and returned, and the `truncated_bytes()` method returns 39.

```C++
    auto json = R"([1,2,3]  {"1":1,"2":3,"4":4} {"key":"intentionally unclosed string  )"_padded;
    simdjson::dom::parser parser;
    simdjson::dom::document_stream stream;
    auto error = parser.parse_many(json,json.size()).get(stream);
    if(error) { std::cerr << error << std::endl; return; }
    for(auto doc : stream) {
       std::cout << doc << std::endl;
    }
    std::cout << stream.truncated_bytes() << " bytes "<< std::endl; // returns 39 bytes
```


Importantly, you should only call `truncated_bytes()` after iterating through all of the documents since the stream cannot tell whether there are truncated documents at the very end when it may not have accessed that part of the data yet.