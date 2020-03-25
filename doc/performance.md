Performance Notes
=================

simdjson strives to be at its fastest *without tuning*, and generally achieves this. However, there
are still some scenarios where tuning can enhance performance.

* [Reusing the parser for maximum efficiency](#reusing-the-parser-for-maximum-efficiency)
  * [Keeping documents around for longer](#keeping-documents-around-for-longer)
* [Server Loops: Long-Running Processes and Memory Capacity](#server-loops-long-running-processes-and-memory-capacity)
* [Large files and huge page support](#large-files-and-huge-page-support)
* [Computed GOTOs](#computed-gotos)

Reusing the parser for maximum efficiency
-----------------------------------------

If you're using simdjson to parse multiple documents, or in a loop, you should make a parser once
and reuse it. The simdjson library will allocate and retain internal buffers between parses, keeping
buffers hot in cache and keeping memory allocation and initialization to a minimum.

```c++
document::parser parser;

// This initializes buffers and a document big enough to handle this JSON.
document &doc = parser.parse("[ true, false ]"_padded);
cout << doc << endl;

// This reuses the existing buffers, and reuses and *overwrites* the old document
doc = parser.parse("[1, 2, 3]"_padded);
cout << doc << endl;

// This also reuses the existing buffers, and reuses and *overwrites* the old document
document &doc2 = parser.parse("true"_padded);
// Even if you keep the old reference around, doc and doc2 refer to the same document.
cout << doc << endl;
cout << doc2 << endl;
```

It's not just internal buffers though. The simdjson library reuses the document itself. Notice that reference?
`document &doc`? That's key. You are only *borrowing* the document from simdjson, which purposely
reuses and overwrites it each time you call parse. This prevent wasteful and unnecessary memory
allocation in 99% of cases where JSON is just read, used, and converted to native values
or thrown away.

> **You are only borrowing the document from the simdjson parser. Don't keep it long term!**

This is key: don't keep the `document&`, `document::element`, `document::array`, `document::object`
or `string_view` objects you get back from the API. Convert them to C++ native values, structs and
arrays that you own.

### Keeping documents around for longer

If you really need to keep parsed JSON documents around for a long time, you can **take** the
document by declaring an actual `document` value.

```c++
document::parser parser;

// This initializes buffers and a document big enough to handle this JSON.
// By casting to document instead of document&, it "steals" the document from the parser so that it
// cannot be overwritten.
document keep_doc = parser.parse("[ true, false ]"_padded);

// This reuses the existing buffers, but initializes a new document.
document &doc = parser.parse("[1, 2, 3]"_padded);

// Now keep_doc and doc refer to different documents.
cout << keep_doc << endl;
cout << doc << endl;
```

If you're using error codes, it can be done like this:

```c++
auto [doc_ref, error] = parser.parse(json); // doc_ref is a document&
if (error) { cerr << error << endl; exit(1); }
document keep_doc = doc_ref; // "steal" the document from the parser
```

This won't allocate anything or copy document memory: instead, it will *steal* the document memory
from the parser. The parser will simply allocate new document memory the next time you call parse.

Server Loops: Long-Running Processes and Memory Capacity
--------------------------------------------------------

The simdjson library automatically expands its memory capacity when larger documents are parsed, so
that you don't unexpectedly fail. In a short process that reads a bunch of files and then exits,
this works pretty flawlessly.

Server loops, though, are long-running processes that will keep the parser around forever. This
means that if you encounter a really, really large document, simdjson will not resize back down.
The simdjson library lets you adjust your allocation strategy to prevent your server from growing
without bound:

* You can set a *max capacity* when constructing a parser:

  ```c++
  document::parser parser(1024*1024); // Never grow past documents > 1MB
  for (web_request request : listen()) {
    auto [doc, error] = parser.parse(request.body);
    // If the document was above our limit, emit 413 = payload too large
    if (error == CAPACITY) { request.respond(413); continue; }
    // ...
  }
  ```

  This parser will grow normally as it encounters larger documents, but will never pass 1MB.

* You can set a *fixed capacity* that never grows, as well, which can be excellent for
  predictability and reliability, since simdjson will never call malloc after startup!

  ```c++
  document::parser parser(0); // This parser will refuse to automatically grow capacity
  parser.set_capacity(1024*1024); // This allocates enough capacity to handle documents <= 1MB
  for (web_request request : listen()) {
    auto [doc, error] = parser.parse(request.body);
    // If the document was above our limit, emit 413 = payload too large
    if (error == CAPACITY) { request.respond(413); continue; }
    // ...
  }
  ```

Large files and huge page support
---------------------------------

There is a memory allocation performance cost the first time you process a large file (e.g. 100MB).
Between the cost of allocation, the fact that the memory is not in cache, and the initial zeroing of
memory, [on some systems, allocation runs far slower than parsing (e.g., 1.4GB/s)](https://lemire.me/blog/2020/01/14/how-fast-can-you-allocate-a-large-block-of-memory-in-c/). Reusing the parser mitigates this by
paying the cost once, but does not eliminate it.

In large file use cases, enabling transparent huge page allocation on the OS can help a lot. We
haven't found the right way to do this on Windows or OS/X, but on Linux, you can enable transparent
huge page allocation with a command like:

```bash
echo always > /sys/kernel/mm/transparent_hugepage/enabled
```

In general, when running benchmarks over large files, we recommend that you report performance
numbers with and without huge pages if possible. Furthermore, you should amortize the parsing (e.g.,
by parsing several large files) to distinguish the time spent parsing from the time spent allocating
memory.

Computed GOTOs
--------------

For best performance, we use a technique called "computed goto" when the compiler supports it, it is
also sometimes described as "Labels as Values". Though it is not part of the C++ standard, it is
supported by many major compilers and it brings measurable performance benefits that are difficult
to achieve otherwise. The computed gotos are  automatically disabled under Visual Studio.

If you wish to forcefully disable computed gotos, you can do so by compiling the code with
`-DSIMDJSON_NO_COMPUTED_GOTO=1`. It is not recommended to disable computed gotos if your compiler
supports it. In fact, you should almost never need to be concerned with computed gotos.

