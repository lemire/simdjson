#include "simdjson.h"

SIMDJSON_PUSH_DISABLE_ALL_WARNINGS

#ifdef SIMDJSON_COMPETITION_YYJSON
#include "yyjson.h"
#endif

#ifdef SIMDJSON_COMPETITION_RAPIDJSON
#include "rapidjson/document.h"
#include "rapidjson/reader.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#endif

#ifdef SIMDJSON_COMPETITION_SAJSON
#include "sajson.h"
#endif

#ifdef SIMDJSON_COMPETITION_NLOHMANN_JSON
#include <nlohmann/json.hpp>
#endif

// This has to be last, for reasons I don't yet understand
#include <benchmark/benchmark.h>

SIMDJSON_POP_DISABLE_WARNINGS

#include "partial_tweets/simdjson_dom.h"
#include "partial_tweets/simdjson_ondemand.h"
#include "partial_tweets/yyjson.h"
#include "partial_tweets/sajson.h"
#include "partial_tweets/rapidjson.h"
#include "partial_tweets/rapidjson_sax.h"
#include "partial_tweets/nlohmann_json.h"
#include "partial_tweets/nlohmann_json_sax.h"

#include "large_random/simdjson_dom.h"
#include "large_random/simdjson_ondemand.h"
#include "large_random/simdjson_ondemand_unordered.h"
#include "large_random/yyjson.h"
#include "large_random/sajson.h"
#include "large_random/rapidjson.h"
#include "large_random/rapidjson_sax.h"
#include "large_random/nlohmann_json.h"
#include "large_random/nlohmann_json_sax.h"

#include "kostya/simdjson_dom.h"
#include "kostya/simdjson_ondemand.h"
#include "kostya/yyjson.h"
#include "kostya/sajson.h"
#include "kostya/rapidjson.h"
#include "kostya/rapidjson_sax.h"
#include "kostya/nlohmann_json.h"
#include "kostya/nlohmann_json_sax.h"

#include "distinct_user_id/simdjson_dom.h"
#include "distinct_user_id/simdjson_dom_json_pointer.h"
#include "distinct_user_id/simdjson_ondemand.h"
#include "distinct_user_id/simdjson_ondemand_json_pointer.h"
#include "distinct_user_id/yyjson.h"
#include "distinct_user_id/sajson.h"
#include "distinct_user_id/rapidjson.h"
#include "distinct_user_id/rapidjson_sax.h"
#include "distinct_user_id/nlohmann_json.h"
#include "distinct_user_id/nlohmann_json_sax.h"

#include "find_tweet/simdjson_dom.h"
#include "find_tweet/simdjson_ondemand.h"
#include "find_tweet/yyjson.h"
#include "find_tweet/sajson.h"
#include "find_tweet/rapidjson.h"
#include "find_tweet/rapidjson_sax.h"
#include "find_tweet/nlohmann_json.h"
#include "find_tweet/nlohmann_json_sax.h"

#include "top_tweet/simdjson_dom.h"
#include "top_tweet/simdjson_ondemand.h"
#include "top_tweet/yyjson.h"
#include "top_tweet/sajson.h"
#include "top_tweet/rapidjson.h"
#include "top_tweet/rapidjson_sax.h"
#include "top_tweet/nlohmann_json.h"
#include "top_tweet/nlohmann_json_sax.h"

BENCHMARK_MAIN();
