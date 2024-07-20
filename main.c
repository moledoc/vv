#include <assert.h>
#include <dirent.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "raylib.h"
#include "raymath.h"

#ifndef UTILS
#define UTILS
#endif // UTILS
#include "utils.h"

#ifndef CIRCULAR_DOUBLY_LINKED_LIST
#define CIRCULAR_DOUBLY_LINKED_LIST
#endif // CIRCULAR_DOUBLY_LINKED_LIST
#include "list.h"

void free_data(void *data) {
  if (data) {
    free((char *)data);
  }
}

bool cmp_data(void *a, void *b) { return strcmp((char *)a, (char *)b) == 0; }

void print_data(CDLLNode *node) {
  printf("(%p) data:'%s' prev:%p next:%p\n", node, (char *)(node->data),
         node->prev, node->next);
}

CDLLNode *get_files(CDLLNode *cursor, char *path) {
  size_t path_size = strlen(path);

  struct stat st;
  assert(stat(path, &st) == 0 && "unexpected error");

  switch (st.st_mode & S_IFMT) {
  case S_IFDIR:
    printf("dir -- %s\n", path);
    break;
  case S_IFREG:
    printf("file -- %s\n", path);
    char *new_path = calloc(path_size + 1, sizeof(char));
    memcpy(new_path, path, path_size);
    if (cursor) {
      cdll_append(cursor, (void *)new_path);
    } else {
      cursor = cdll_create(cmp_data, print_data, free_data, NULL, NULL,
                           (void *)new_path);
    }
    return cursor;
  default:
    printf("unsupported filetype: %u\n", st.st_mode & S_IFMT); // see stat(2)
    return cursor;
  }

  DIR *dp;
  struct dirent *ep;
  dp = opendir(path);
  assert(dp && "unexpected NULL");

  while ((ep = readdir(dp))) {
    if (strcmp(".", ep->d_name) == 0 || strcmp("..", ep->d_name) == 0) {
      continue;
    }

    size_t ep_name_len = strlen(ep->d_name);
    size_t new_path_size = path_size + ep_name_len + 1;
    char *new_path = calloc(new_path_size + 1, sizeof(char));
    new_path[new_path_size] = '\0';

    memcpy(new_path, path, path_size);
    new_path[path_size] = '/';
    memcpy(new_path + (path_size + 1), ep->d_name, ep_name_len);

    switch (ep->d_type) {
    case DT_REG:
      printf("file - %s\n", new_path);
      if (cursor) {
        cdll_append(cursor, (void *)new_path);
      } else {
        cursor = cdll_create(cmp_data, print_data, free_data, NULL, NULL,
                             (void *)new_path);
      }
      break;
    }
  }
  (void)closedir(dp);
  return cursor;
}

int main(int argc, char **argv) {

  char *prog_name = shift(&argc, &argv);

  CDLLNode *cdll = NULL;
  for (; argc;) {
    char *arg = shift(&argc, &argv);
    cdll = get_files(cdll, arg);
  }
  cdll_list(cdll);
  cdll_nodes_free(cdll);

  return 0;

  InitWindow(800, 600, prog_name);
  SetTargetFPS(30);
  SetWindowMonitor(0);
  SetWindowPosition(GetMonitorWidth(0) / 2 - 400,
                    GetMonitorHeight(0) / 2 - 300);
  SetWindowState(FLAG_WINDOW_RESIZABLE);
  SetExitKey(KEY_Q);

  Image img = LoadImage("./test2.png");
  assert(IsImageReady(img));

  Texture2D texture = LoadTextureFromImage(img);
  SetWindowSize(texture.width, texture.height);

  float zoom = 1;
  Vector2 target = {GetScreenWidth() * 0.5, GetScreenHeight() * 0.5};
  Vector2 offset = {GetScreenWidth() * 0.5, GetScreenHeight() * 0.5};
  Vector2 mouse = {0};
  Vector2 mouse_offset = {0};

  while (!WindowShouldClose()) {
    BeginDrawing();
    {
      ClearBackground(BLACK);

      Camera2D cam = {
          .target = target,
          .zoom = zoom,
          .offset = offset,
      };

      float wheel = GetMouseWheelMove();
      if (wheel > 0) {
        zoom *= 1.1;
        mouse = GetScreenToWorld2D(GetMousePosition(), cam);
        if (!mouse_offset.x && !mouse_offset.y) {
          mouse_offset = mouse;
        }
        target = Vector2Subtract(target, mouse);
        target = Vector2Add(target, mouse_offset);
      } else if (wheel < 0) {
        zoom *= 0.9;
        mouse = GetScreenToWorld2D(GetMousePosition(), cam);
        if (!mouse_offset.x && !mouse_offset.y) {
          mouse_offset = mouse;
        }
        target = Vector2Subtract(target, mouse);
        target = Vector2Add(target, mouse_offset);

      } else if (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE)) {
        mouse = GetScreenToWorld2D(GetMousePosition(), cam);
        if (!mouse_offset.x && !mouse_offset.y) {
          mouse_offset = mouse;
        }
        target = Vector2Subtract(target, mouse);
        target = Vector2Add(target, mouse_offset);

      } else {
        mouse_offset = (Vector2){0};
      }

      cam.target = target;
      cam.zoom = zoom;

      BeginMode2D(cam);
      { DrawTexture(texture, 0, 0, WHITE); }
      EndMode2D();
    }
    EndDrawing();
  }
  UnloadImage(img);
  UnloadTexture(texture);
}