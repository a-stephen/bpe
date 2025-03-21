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

// Freq *freq = NULL;

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
  }
  printf("\n");
}

#define swap(Type, x, y) \
  do { \
    Type t = (x); \
    (x) = (y); \
    (y) = t; \
  } while(0)


bool dump_pairs(const char *file_path, Pairs pairs) {
  return write_entire_file(file_path, pairs.items, pairs.count*sizeof(*pairs.items));
}

bool load_pairs(const char *file_path, Pairs *pairs, String_Builder *sb) {
  if (!read_entire_file(file_path, sb)) return false;
  if (sb->count%sizeof(*pairs->items) != 0) {
    fprintf(stderr, "ERROR: size of %s (%zu) must be divisibile by %zu\n", file_path, sb->count, sizeof(*pairs->items));
    return false;
  }
  Pair *items = (void*)sb->items;
  size_t items_count = sb->count/sizeof(*pairs->items);
  for (size_t i = 0; i < items_count; ++i) {
    da_append(pairs, items[i]);
  }
  return true;
}

void generate_dots(Pairs pairs) {
  printf("digraph Pairs {\n"); 
  for (uint32_t token = 0; token < pairs.count; ++token) {
    if (token != pairs.items[token].l) {
      printf("  %u -> %u\n", token, pairs.items[token].l);
      printf("  %u -> %u\n", token, pairs.items[token].r);
    }
  }
  printf("}\n");
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

  for (;;) {
	  // printf("%zu\n", tokens.count);
	  // render_tokens(pairs, tokens);
	  hmfree(freq);
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
          if (freq[max_index].value <= 1) break; // compression done!!
	  da_append(&pairs, freq[max_index].key);
	  tokens_out.count = 0;
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
	  swap(Tokens, tokens, tokens_out);
  }
  // printf("Generated %zu tokens", pairs.count);
  generate_dots(pairs);
  if (!dump_pairs("pair.bin", pairs)) return 1;
  return 0;
}
