/*
 * Word count application with one thread per input file.
 *
 * You may modify this file in any way you like, and are expected to modify it.
 * Your solution must read each input file from a separate thread. We encourage
 * you to make as few changes as necessary.
 */

/*
 * Copyright © 2021 University of California, Berkeley
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <pthread.h>

#include "word_count.h"
#include "word_helpers.h"

typedef struct thread_arg {
  const char* path;
  word_count_list_t* wclist;
} thread_arg_t;

static void* worker(void* arg) {
  thread_arg_t* ta = (thread_arg_t*)arg;
  FILE* f = fopen(ta->path, "r");
  if (f != NULL) {
    count_words(ta->wclist, f);
    fclose(f);
  }
  return NULL;
}

/*
 * main - handle command line, spawning one thread per file.
 */
int main(int argc, char* argv[]) {
  /* Create the empty data structure. */
  word_count_list_t word_counts;
  init_words(&word_counts);

  if (argc <= 1) {
    /* Process stdin in a single thread. */
    count_words(&word_counts, stdin);
  } else {
    int n = argc - 1;
    pthread_t threads[n];
    int created[n];
    thread_arg_t args[n];

    for (int i = 0; i < n; i++) {
      created[i] = 0;
      args[i].path = argv[i + 1];
      args[i].wclist = &word_counts;
      if (pthread_create(&threads[i], NULL, worker, &args[i]) == 0) {
        created[i] = 1;
      } else {
        /* Fallback: process this file in the main thread if thread creation fails. */
        FILE* f = fopen(args[i].path, "r");
        if (f != NULL) {
          count_words(&word_counts, f);
          fclose(f);
        }
      }
    }

    for (int i = 0; i < n; i++) {
      if (created[i]) {
        pthread_join(threads[i], NULL);
      }
    }
  }

  /* Output final result of all threads' work. */
  wordcount_sort(&word_counts, less_count);
  fprint_words(&word_counts, stdout);
  return 0;
}
