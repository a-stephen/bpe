#include "stdio.h"
#include "string.h"
#include "stdint.h"

#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX
#include "nob.h"

#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"

typedef struct {
  uint32_t l, r;
} Pair;

typedef struct {
  Pair *items;
  size_t count;
  size_t capacity;
} Pairs;

typedef struct {
  Pair key;
  size_t value;
} Freq;

typedef struct {
  uint32_t *items;
  size_t count;
  size_t capacity;
} Tokens;

typedef struct {
  Freq *items;
  size_t count;
  size_t capacity;
} Freqs;

Freq *freq = NULL;

int compare_freqs(const void *a, const void *b) {
  const Freq *af = a;
  const Freq *bf = b;
  return (int)bf->value - (int)af->value;
}

void render_tokens(Pairs pairs, Tokens tokens) {
  for (size_t i = 0; i < tokens.count; ++i) {
    uint32_t token = tokens.items[i];
    assert(token < pairs.count);
    if (pairs.items[token].l == token) {
      printf("%c", token);
    } else {
      printf("[%u]", token);
    }
    // printf("\n");
  }
  printf("\n");
}

int main()
{
  // Taken from the original wikipedia article.
  const char *text = "The original BPE algorithm operates by iteratively replacing the most common contiguous sequences of characters in a target text with unused 'placeholder' bytes. The iteration ends when no sequences can be found, leaving the target text effectively compressed. Decompression can be performed by reversing this process, querying known placeholder terms against their corresponding denoted sequence, using a lookup table. In the original paper, this lookup table is encoded and stored alongside the compressed text.";
  int text_size = strlen(text);
  // 0 => {.l = 0, .r = ??} Setting the left to itself and r stays undefined.
  // 1 => {.l = 1, .r = ??}
  // ....
  // 256 => {.l = 256, .r = ??}
  // 256 => {.l = 69, .r = 120}
  Freq *freq = NULL;
  Pairs pairs = {};
  Tokens tokens = {0};
  Tokens tokens_out = {0};

  for (uint32_t i = 0; i < 256; ++i) {
    Pair p = {.l = i};
    da_append(&pairs, p);
  }

  /*printf("(%u, %u)\n", pairs.items[17].l, pairs.items[0].r);*/

  for (int i = 0; i < text_size; ++i) {
    da_append(&tokens, text[i]);
  } 

  for (size_t i = 0; i < tokens.count - 1; ++i) {
    Pair pair = {
      .l = tokens.items[i],
      .r = tokens.items[i+1]
    };
    ptrdiff_t d = hmgeti(freq, pair);
    if (d < 0) hmput(freq, pair, 1);
    else freq[d].value += 1;
    // hmput(freq, key, count + 1)
  }

  ptrdiff_t max_index = 0;
  for (ptrdiff_t i = 1; i < hmlen(freq); ++i) {
      if (freq[i].value > freq[max_index].value) {
        max_index = i;
      }
  }
  
  da_append(&pairs, freq[max_index].key);
  
  for (size_t i = 0; i < tokens.count; ) {
    if (i + 1 >= tokens.count) {
      da_append(&tokens_out, tokens.items[i]);
      i += 1;
    } else {
      Pair pair = {
        .l = tokens.items[i],
        .r = tokens.items[i + 1]
      };
      if (memcmp(&pair, &freq[max_index].key, sizeof(pair)) == 0) {
        da_append(&tokens_out, pairs.count -1);
	i += 2;
      } else {
        da_append(&tokens_out, tokens.items[i]);
	i += 1;
      }
    }
  }
  render_tokens(pairs, tokens_out);
  /*Freqs sorted_freqs = {0};*/
  /**/
  /*for (ptrdiff_t i = 0; i < hmlen(freq); ++i) {*/
  /*  // printf("%c%c => %zu\n", freq[i].key.pair[0], freq[i].key.pair[1], freq[i].value);*/
  /*  da_append(&sorted_freqs, freq[i]);*/
  /*}*/
  /*qsort(sorted_freqs.items, sorted_freqs.count, sizeof(*sorted_freqs.items), compare_freqs);*/
  /*for (size_t i = 0; i < 10; ++i) {*/
  /*  Freq *freq = &sorted_freqs.items[i];*/
  /*  printf("(%u, %u) => %zu\n", freq->key.l, freq->key.r, freq->value);*/
  /*}*/
  return 0;
}
