/* auto-generated on Thu Mar  7 17:39:02 PST 2019. Do not edit! */

#include <iostream>
#include "simdjson.h"
#include "simdjson.cpp"
int main(int argc, char *argv[]) {
  const char * filename = argv[1];
  std::string_view p = get_corpus(filename);
  ParsedJson pj = build_parsed_json(p); // do the parsing
  if( ! pj.isValid() ) {
    std::cout << "not valid" << std::endl;
  } else {
    std::cout << "valid" << std::endl;
  }
  return EXIT_SUCCESS;
}

