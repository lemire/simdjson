#pragma once

#if SIMDJSON_EXCEPTIONS

#include "distinct_user_id.h"

namespace distinct_user_id {

using namespace simdjson;

struct simdjson_ondemand_json_pointer {
  ondemand::parser parser{};

  bool run(simdjson::padded_string &json, std::vector<uint64_t> &result) {
    // Walk the document, parsing as we go
    auto doc = parser.iterate(json);
    for (ondemand::object tweet : doc.find_field("statuses")) {
      // We believe that all statuses have a matching
      // user, and we are willing to throw when they do not.
      result.push_back(tweet.at_pointer("/user/id"));
      // Not all tweets have a "retweeted_status", but when they do
      // we want to go and find the user within.
      auto retweet_id = tweet.at_pointer("/retweeted_status/user/id");
      if (retweet_id.error() != NO_SUCH_FIELD) {
        result.push_back(retweet_id);
      }
    }

    return true;
  }
};

BENCHMARK_TEMPLATE(distinct_user_id, simdjson_ondemand_json_pointer)->UseManualTime();

} // namespace distinct_user_id

#endif // SIMDJSON_EXCEPTIONS
