#pragma once

// {
#ifdef UTILS // HEADER
// shift consumes one argv elem
// returns NULL if all elems are consumed
char *shift(int *argc, char ***argv);

#endif // UTILS // HEADER
// }

// {
#ifdef UTILS // IMPLEMENTATION

char *shift(int *argc, char ***argv) {
  if (*argc <= 0) {
    return NULL;
  }
  char *result = **argv;
  *argc -= 1;
  *argv += 1;
  return result;
}

#endif // UTILS // IMPLEMENTATION

// }