namespace simdjson {
namespace SIMDJSON_IMPLEMENTATION {
namespace ondemand {
namespace logger {

static constexpr const char * DASHES = "----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------";
static constexpr const int LOG_EVENT_LEN = 20;
static constexpr const int LOG_BUFFER_LEN = 30;
static constexpr const int LOG_SMALL_BUFFER_LEN = 10;
static int log_depth = 0; // Not threadsafe. Log only.

// Helper to turn unprintable or newline characters into spaces
static inline char printable_char(char c) {
  if (c >= 0x20) {
    return c;
  } else {
    return ' ';
  }
}

inline void log_event(const json_iterator &iter, const char *type, std::string_view detail, int delta, int depth_delta) noexcept {
  log_line(iter, "", type, detail, delta, depth_delta);
}

inline void log_value(const json_iterator &iter, token_position index, depth_t depth, const char *type, std::string_view detail) noexcept {
  log_line(iter, index, depth, "", type, detail);
}
inline void log_value(const json_iterator &iter, const char *type, std::string_view detail, int delta, int depth_delta) noexcept {
  log_line(iter, "", type, detail, delta, depth_delta);
}

inline void log_start_value(const json_iterator &iter, token_position index, depth_t depth, const char *type, std::string_view detail) noexcept {
  log_line(iter, index, depth, "+", type, detail);
  if (LOG_ENABLED) { log_depth++; }
}
inline void log_start_value(const json_iterator &iter, const char *type, int delta, int depth_delta) noexcept {
  log_line(iter, "+", type, "", delta, depth_delta);
  if (LOG_ENABLED) { log_depth++; }
}

inline void log_end_value(const json_iterator &iter, const char *type, int delta, int depth_delta) noexcept {
  if (LOG_ENABLED) { log_depth--; }
  log_line(iter, "-", type, "", delta, depth_delta);
}

inline void log_error(const json_iterator &iter, const char *error, const char *detail, int delta, int depth_delta) noexcept {
  log_line(iter, "ERROR: ", error, detail, delta, depth_delta);
}
inline void log_error(const json_iterator &iter, token_position index, depth_t depth, const char *error, const char *detail) noexcept {
  log_line(iter, index, depth, "ERROR: ", error, detail);
}

inline void log_event(const value_iterator &iter, const char *type, std::string_view detail, int delta, int depth_delta) noexcept {
  log_event(iter.json_iter(), type, detail, delta, depth_delta);
}

inline void log_value(const value_iterator &iter, const char *type, std::string_view detail, int delta, int depth_delta) noexcept {
  log_value(iter.json_iter(), type, detail, delta, depth_delta);
}

inline void log_start_value(const value_iterator &iter, const char *type, int delta, int depth_delta) noexcept {
  log_start_value(iter.json_iter(), type, delta, depth_delta);
}

inline void log_end_value(const value_iterator &iter, const char *type, int delta, int depth_delta) noexcept {
  log_end_value(iter.json_iter(), type, delta, depth_delta);
}

inline void log_error(const value_iterator &iter, const char *error, const char *detail, int delta, int depth_delta) noexcept {
  log_error(iter.json_iter(), error, detail, delta, depth_delta);
}

inline void log_headers() noexcept {
  if (LOG_ENABLED) {
    // Technically a static variable is not thread-safe, but if you are using threads
    // and logging... well...
    static bool displayed_hint{false};
    log_depth = 0;
    printf("\n");
    if(!displayed_hint) {
      // We only print this helpful header once.
      printf("# Logging provides the depth and position of the iterator user-visible steps:\n");
      printf("# +array says 'this is where we were when we discovered the start array'\n");
      printf("# -array says 'this is where we were when we ended the array'\n");
      printf("# skip says 'this is a structural or value I am skipping'\n");
      printf("# +/-skip says 'this is a start/end array or object I am skipping'\n");
      printf("#\n");
      printf("# The identation of the terms (array, string,...) indicates the depth,\n");
      printf("# in addition to the depth being displayed.\n");
      printf("#\n");
      printf("# Every token in the document has a single depth determined by the tokens before it,\n");
      printf("# and is not affected by what the token actually is.\n");
      printf("#\n");
      printf("# Not all structural elements are presented as tokens in the logs.\n");
      printf("#\n");
      printf("# We never give control to the user within an empty array or an empty object.\n");
      printf("#\n");
      printf("# Inside an array, having a depth greater than the array's depth means that\n");
      printf("# we are pointing inside a value.\n");
      printf("# Having a depth equal to the array means that we are pointing right before a value.\n");
      printf("# Having a depth smaller than the array means that we have moved beyond the array.\n");
      displayed_hint = true;
    }
    printf("\n");
    printf("| %-*s ", LOG_EVENT_LEN,        "Event");
    printf("| %-*s ", LOG_BUFFER_LEN,       "Buffer");
    printf("| %-*s ", LOG_SMALL_BUFFER_LEN, "Next");
    // printf("| %-*s ", 5,                    "Next#");
    printf("| %-*s ", 5,                    "Depth");
    printf("| Detail ");
    printf("|\n");

    printf("|%.*s", LOG_EVENT_LEN+2, DASHES);
    printf("|%.*s", LOG_BUFFER_LEN+2, DASHES);
    printf("|%.*s", LOG_SMALL_BUFFER_LEN+2, DASHES);
    // printf("|%.*s", 5+2, DASHES);
    printf("|%.*s", 5+2, DASHES);
    printf("|--------");
    printf("|\n");
    fflush(stdout);
  }
}

inline void log_line(const json_iterator &iter, const char *title_prefix, const char *title, std::string_view detail, int delta, int depth_delta) noexcept {
  log_line(iter, iter.position()+delta, depth_t(iter.depth()+depth_delta), title_prefix, title, detail);
}
inline void log_line(const json_iterator &iter, token_position index, depth_t depth, const char *title_prefix, const char *title, std::string_view detail) noexcept {
  if (LOG_ENABLED) {
    const int indent = depth*2;
    const auto buf = iter.token.buf;
    printf("| %*s%s%-*s ",
      indent, "",
      title_prefix,
      LOG_EVENT_LEN - indent - int(strlen(title_prefix)), title
      );
    {
      // Print the current structural.
      printf("| ");
      auto current_structural = &buf[*index];
      for (int i=0;i<LOG_BUFFER_LEN;i++) {
        printf("%c", printable_char(current_structural[i]));
      }
      printf(" ");
    }
    {
      // Print the next structural.
      printf("| ");
      auto next_structural = &buf[*(index+1)];
      for (int i=0;i<LOG_SMALL_BUFFER_LEN;i++) {
        printf("%c", printable_char(next_structural[i]));
      }
      printf(" ");
    }
    // printf("| %5u ", *(index+1));
    printf("| %5u ", depth);
    printf("| %.*s ", int(detail.size()), detail.data());
    printf("|\n");
    fflush(stdout);
  }
}

} // namespace logger
} // namespace ondemand
} // namespace SIMDJSON_IMPLEMENTATION
} // namespace simdjson
