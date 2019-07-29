
// This is included *multiple times,* once per architecture, from stage1_find_marks.cpp

#define S2(x) #x
#define S(x) S2(x)
#pragma message("Including " S(IMPL_TARGET_REGION) " / " S(IMPL_ARCHITECTURE))


IMPL_TARGET_REGION
namespace simdjson {

  // return a bitvector indicating where we have characters that end an odd-length
  // sequence of backslashes (and thus change the behavior of the next character
  // to follow). A even-length sequence of backslashes, and, for that matter, the
  // largest even-length prefix of our odd-length sequence of backslashes, simply
  // modify the behavior of the backslashes themselves.
  // We also update the prev_iter_ends_odd_backslash reference parameter to
  // indicate whether we end an iteration on an odd-length sequence of
  // backslashes, which modifies our subsequent search for odd-length
  // sequences of backslashes in an obvious way.
  // We need to compile that code for multiple architectures. However, target attributes can be used
  // only once by function definition. Huge macro seemed better than huge code duplication.
  // uint64_t FIND_ODD_BACKSLASH_SEQUENCES(architecture T, simd_input<T> in, uint64_t &prev_iter_ends_odd_backslash)
  template<> really_inline
  uint64_t find_odd_backslash_sequences<IMPL_ARCHITECTURE>(simd_input<IMPL_ARCHITECTURE> in, uint64_t & prev_iter_ends_odd_backslash) {
    const uint64_t even_bits = 0x5555555555555555ULL;
    const uint64_t odd_bits = ~even_bits;
    uint64_t bs_bits = cmp_mask_against_input<IMPL_ARCHITECTURE>(in, '\\');
    uint64_t start_edges = bs_bits & ~(bs_bits << 1);
    /* flip lowest if we have an odd-length run at the end of the prior        */
    /* iteration                                                               */
    uint64_t even_start_mask = even_bits ^ prev_iter_ends_odd_backslash;
    uint64_t even_starts = start_edges & even_start_mask;
    uint64_t odd_starts = start_edges & ~even_start_mask;
    uint64_t even_carries = bs_bits + even_starts;

    uint64_t odd_carries;
    /* must record the carry-out of our odd-carries out of bit 63; this        */
    /* indicates whether the sense of any edge going to the next iteration     */
    /* should be flipped                                                       */
    bool iter_ends_odd_backslash =
        add_overflow(bs_bits, odd_starts, &odd_carries);

    odd_carries |=
        prev_iter_ends_odd_backslash;  /* push in bit zero as a potential end  */
                                      /* if we had an odd-numbered run at the */
                                      /* end of the previous iteration        */
    prev_iter_ends_odd_backslash = iter_ends_odd_backslash ? 0x1ULL : 0x0ULL;
    uint64_t even_carry_ends = even_carries & ~bs_bits;
    uint64_t odd_carry_ends = odd_carries & ~bs_bits;
    uint64_t even_start_odd_end = even_carry_ends & odd_bits;
    uint64_t odd_start_even_end = odd_carry_ends & even_bits;
    uint64_t odd_ends = even_start_odd_end | odd_start_even_end;
    return odd_ends;
  }

  // return both the quote mask (which is a half-open mask that covers the first
  // quote
  // in an unescaped quote pair and everything in the quote pair) and the quote
  // bits, which are the simple
  // unescaped quoted bits. We also update the prev_iter_inside_quote value to
  // tell the next iteration
  // whether we finished the final iteration inside a quote pair; if so, this
  // inverts our behavior of
  // whether we're inside quotes for the next iteration.
  // Note that we don't do any error checking to see if we have backslash
  // sequences outside quotes; these
  // backslash sequences (of any length) will be detected elsewhere.
  // We need to compile that code for multiple architectures. However, target attributes can be used
  // only once by function definition. Huge macro seemed better than huge code duplication.
  // uint64_t FIND_QUOTE_MASK_AND_BITS(architecture T, simd_input<T> in, uint64_t odd_ends,
  //    uint64_t &prev_iter_inside_quote, uint64_t &quote_bits, uint64_t &error_mask)
  template<> really_inline
  uint64_t find_quote_mask_and_bits<IMPL_ARCHITECTURE>(simd_input<IMPL_ARCHITECTURE> in, uint64_t odd_ends, uint64_t & prev_iter_inside_quote, uint64_t & quote_bits, uint64_t & error_mask) {
    quote_bits = cmp_mask_against_input<IMPL_ARCHITECTURE>(in, '"');
    quote_bits = quote_bits & ~odd_ends;
    uint64_t quote_mask = compute_quote_mask<IMPL_ARCHITECTURE>(quote_bits);
    quote_mask ^= prev_iter_inside_quote;
    /* All Unicode characters may be placed within the                                                */
    /* quotation marks, except for the characters that MUST be escaped:                               */
    /* quotation mark, reverse solidus, and the control characters (U+0000                            */
    /*through U+001F).                                                                                */
    /* https://tools.ietf.org/html/rfc8259                                                            */
    uint64_t unescaped = unsigned_lteq_against_input<IMPL_ARCHITECTURE>(in, 0x1F);
    error_mask |= quote_mask & unescaped;
    /* right shift of a signed value expected to be well-defined and standard                         */
    /* compliant as of C++20,                                                                         */
    /* John Regher from Utah U. says this is fine code                                                */
    prev_iter_inside_quote =
        static_cast<uint64_t>(static_cast<int64_t>(quote_mask) >> 63);
    return quote_mask;
  }

  template<>
  int find_structural_bits<IMPL_ARCHITECTURE>(const uint8_t *buf, size_t len, ParsedJson &pj) {
    if (len > pj.bytecapacity) {
      std::cerr << "Your ParsedJson object only supports documents up to "
          << pj.bytecapacity << " bytes but you are trying to process " << len
          << " bytes" << std::endl;
      return CAPACITY;
    }
    uint32_t *base_ptr = pj.structural_indexes;
    uint32_t base = 0;
    utf8_checking_state<IMPL_ARCHITECTURE> state;

    /* we have padded the input out to 64 byte multiple with the remainder being                   */
    /* zeros                                                                                       */

    /* persistent state across loop                                                                */
    /* does the last iteration end with an odd-length sequence of backslashes?                     */
    /* either 0 or 1, but a 64-bit value                                                           */
    uint64_t prev_iter_ends_odd_backslash = 0ULL;
    /* does the previous iteration end inside a double-quote pair?                                 */
    uint64_t prev_iter_inside_quote = 0ULL;  /* either all zeros or all ones                       */
    /* does the previous iteration end on something that is a predecessor of a                     */
    /* pseudo-structural character - i.e. whitespace or a structural character                     */
    /* effectively the very first char is considered to follow "whitespace" for                    */
    /* the                                                                                         */
    /* purposes of pseudo-structural character detection so we initialize to 1                     */
    uint64_t prev_iter_ends_pseudo_pred = 1ULL;

    /* structurals are persistent state across loop as we flatten them on the                      */
    /* subsequent iteration into our array pointed to be base_ptr.                                 */
    /* This is harmless on the first iteration as structurals==0                                   */
    /* and is done for performance reasons; we can hide some of the latency of the                 */
    /* expensive carryless multiply in the previous step with this work                            */
    uint64_t structurals = 0;

    size_t lenminus64 = len < 64 ? 0 : len - 64;
    size_t idx = 0;
    uint64_t error_mask = 0; /* for unescaped characters within strings (ASCII code points < 0x20) */

    for (; idx < lenminus64; idx += 64) {

      simd_input<IMPL_ARCHITECTURE> in = fill_input<IMPL_ARCHITECTURE>(buf+idx);
      check_utf8<IMPL_ARCHITECTURE>(in, state);
      /* detect odd sequences of backslashes                                                       */
      uint64_t odd_ends = find_odd_backslash_sequences<IMPL_ARCHITECTURE>(
          in, prev_iter_ends_odd_backslash);

      /* detect insides of quote pairs ("quote_mask") and also our quote_bits                      */
      /* themselves                                                                                */
      uint64_t quote_bits;
      uint64_t quote_mask = find_quote_mask_and_bits<IMPL_ARCHITECTURE>(
          in, odd_ends, prev_iter_inside_quote, quote_bits, error_mask);

      /* take the previous iterations structural bits, not our current iteration,                  */
      /* and flatten                                                                               */
      flatten_bits(base_ptr, base, idx, structurals);

      uint64_t whitespace;
      find_whitespace_and_structurals<IMPL_ARCHITECTURE>(in, whitespace, structurals);

      /* fixup structurals to reflect quotes and add pseudo-structural characters                  */
      structurals = finalize_structurals(structurals, whitespace, quote_mask,
                                        quote_bits, prev_iter_ends_pseudo_pred);
    }

    /*//////////////                                                                               */
    /*/ we use a giant copy-paste which is ugly.                                                   */
    /*/ but otherwise the string needs to be properly padded or else we                            */
    /*/ risk invalidating the UTF-8 checks.                                                        */
    /*//////////                                                                                   */
    if (idx < len) {
      uint8_t tmpbuf[64];
      memset(tmpbuf, 0x20, 64);
      memcpy(tmpbuf, buf + idx, len - idx);
      simd_input<IMPL_ARCHITECTURE> in = fill_input<IMPL_ARCHITECTURE>(tmpbuf);
      check_utf8<IMPL_ARCHITECTURE>(in, state);

      /* detect odd sequences of backslashes                                                       */
      uint64_t odd_ends = find_odd_backslash_sequences<IMPL_ARCHITECTURE>(
          in, prev_iter_ends_odd_backslash);

      /* detect insides of quote pairs ("quote_mask") and also our quote_bits                      */
      /* themselves                                                                                */
      uint64_t quote_bits;
      uint64_t quote_mask = find_quote_mask_and_bits<IMPL_ARCHITECTURE>(
          in, odd_ends, prev_iter_inside_quote, quote_bits, error_mask);

      /* take the previous iterations structural bits, not our current iteration,                  */
      /* and flatten                                                                               */
      flatten_bits(base_ptr, base, idx, structurals);

      uint64_t whitespace;
      find_whitespace_and_structurals<IMPL_ARCHITECTURE>(in, whitespace, structurals);

      /* fixup structurals to reflect quotes and add pseudo-structural characters                  */
      structurals = finalize_structurals(structurals, whitespace, quote_mask,
                                        quote_bits, prev_iter_ends_pseudo_pred);
      idx += 64;
    }

    /* is last string quote closed?                                                                */
    if (prev_iter_inside_quote) {
        return UNCLOSED_STRING;
    }

    /* finally, flatten out the remaining structurals from the last iteration                      */
    flatten_bits(base_ptr, base, idx, structurals);

    pj.n_structural_indexes = base;
    /* a valid JSON file cannot have zero structural indexes - we should have                      */
    /* found something                                                                             */
    if (pj.n_structural_indexes == 0u) {
      return EMPTY;
    }
    if (base_ptr[pj.n_structural_indexes - 1] > len) {
      return UNEXPECTED_ERROR;
    }
    if (len != base_ptr[pj.n_structural_indexes - 1]) {
      /* the string might not be NULL terminated, but we add a virtual NULL ending                 */
      /* character.                                                                                */
      base_ptr[pj.n_structural_indexes++] = len;
    }
    /* make it safe to dereference one beyond this array                                           */
    base_ptr[pj.n_structural_indexes] = 0;
    if (error_mask) {
      return UNESCAPED_CHARS;
    }
    return check_utf8_errors<IMPL_ARCHITECTURE>(state);
  }                                                                                                   
}
UNTARGET_REGION