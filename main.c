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
  printf("(%p) data:'%s' prev:%p next:%p\n", (void *)node, (char *)(node->data),
         (void *)node->prev, (void *)node->next);
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
    if (strstr(path, ".png") == NULL && strstr(path, ".jpg") == NULL) {
      printf("ignoring %s\n", path);
      return cursor;
    }
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
    if (strstr(ep->d_name, ".png") == NULL &&
        strstr(ep->d_name, ".jpg") == NULL) {
      printf("ignoring %s\n", ep->d_name);
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

#define M_WIDTH (GetMonitorWidth(0))
#define M_HEIGHT (GetMonitorHeight(0))
#define W_WIDTH M_WIDTH
#define W_HEIGHT M_HEIGHT

void load_image(CDLLNode *img_paths, Image *img, Texture2D *texture) {
  assert(img && texture && "unexpected NULL");
  UnloadImage(*img);
  UnloadTexture(*texture);
  *img = LoadImage((char *)img_paths->data);
  assert(IsImageReady(*img));
  *texture = LoadTextureFromImage(*img);
  SetWindowPosition(0, 0);
}

void viewer(char *prog_name, CDLLNode *img_paths) {
  if (!img_paths) {
    return;
  }
  InitWindow(W_WIDTH, W_HEIGHT, prog_name);
  SetTargetFPS(30);
  SetWindowMonitor(0);

  SetWindowState(FLAG_WINDOW_RESIZABLE);
  SetExitKey(KEY_Q);

  float zoom_orig = 1;
  float zoom = 1;
  Vector2 target_orig = {GetScreenWidth() * 0.5, GetScreenHeight() * 0.5};
  Vector2 target = target_orig;
  Vector2 offset = {GetScreenWidth() * 0.5, GetScreenHeight() * 0.5};
  Vector2 mouse = {0};
  Vector2 mouse_offset = {0};
  Image *img = calloc(1, sizeof(Image));
  Texture2D texture = {0};
  size_t image_count = cdll_list_len(img_paths);
  size_t image_cursor = 0;
  char text[256];

  load_image(img_paths, img, &texture);

  while (!WindowShouldClose()) {
    BeginDrawing();
    {
      ClearBackground(BLACK);

      assert(texture.id >= 0 && "unexpected empty");

      Camera2D cam = {
          .target = target,
          .zoom = zoom,
          .offset = offset,
      };

      float wheel = GetMouseWheelMove();
      if (wheel > 0) {
        zoom += 0.1;
        mouse = GetScreenToWorld2D(GetMousePosition(), cam);
        if (!mouse_offset.x && !mouse_offset.y) {
          mouse_offset = mouse;
        }
        target = Vector2Subtract(target, mouse);
        target = Vector2Add(target, mouse_offset);
      } else if (wheel < 0) {
        zoom -= 0.1;
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

      } else if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        image_cursor = (image_cursor - 1) % image_count;
        if (image_cursor < 0) {
          image_cursor = image_count - image_cursor;
        }
        img_paths = img_paths->prev;
        load_image(img_paths, img, &texture);
        zoom = zoom_orig;
        target = target_orig;
      } else if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
        image_cursor = (image_cursor + 1) % image_count;
        img_paths = img_paths->next;
        load_image(img_paths, img, &texture);
        zoom = zoom_orig;
        target = target_orig;
      } else {
        mouse_offset = (Vector2){0};
      }

      cam.target = target;
      cam.zoom = zoom;

      BeginMode2D(cam);
      {
        DrawTexture(texture, 0, 0, WHITE);
        memset(text, '\0', sizeof(text));
        snprintf(text, sizeof(text), "%lu/%lu\t%s", image_cursor + 1,
                 image_count, (char *)img_paths->data);
        DrawText(text, 0, 0, 16, BLACK);
      }
      EndMode2D();
    }
    EndDrawing();
  }
  if (img) {
    free(img);
  }
  CloseWindow();
  return;
}

int main(int argc, char **argv) {

  char *prog_name = shift(&argc, &argv);

  CDLLNode *cdll = NULL;
  for (; argc;) {
    char *arg = shift(&argc, &argv);
    cdll = get_files(cdll, arg);
  }
  cdll_list(cdll);
  viewer(prog_name, cdll);
  cdll_nodes_free(cdll);
}