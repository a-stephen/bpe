#include "stdio.h"
#include "string.h"

#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"

typedef struct {
  char pair[2];
} Pair;

typedef struct {
  Pair key;
  size_t value;
} KV;

KV *freq = NULL;

int main()
{
  // Taken from the original wikipedia article.
  const char *text = "The original BPE algorithm operates by iteratively replacing the most common contiguous sequences of characters in a target text with unused 'placeholder' bytes. The iteration ends when no sequences can be found, leaving the target text effectively compressed. Decompression can be performed by reversing this process, querying known placeholder terms against their corresponding denoted sequence, using a lookup table. In the original paper, this lookup table is encoded and stored alongside the compressed text.";
  int text_size = strlen(text);
  for (int i = 0; i < text_size - 1; ++i) {
    Pair pair = {
      .pair = {text[i], text[i+1]}
    };
    ptrdiff_t d = hmgeti(freq, pair);
    if (d < 0) hmput(freq, pair, 1);
    else freq[d].value += 1;
    // hmput(freq, key, count + 1)
  }

  for (ptrdiff_t i = 0; i < hmlen(freq); ++i) {
    printf("%c%c => %zu\n", freq[i].key.pair[0], freq[i].key.pair[1], freq[i].value);
  }
  return 0;
}
