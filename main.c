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

#define M_WIDTH (GetMonitorWidth(0))
#define M_HEIGHT (GetMonitorHeight(0))
#define W_WIDTH M_WIDTH
#define W_HEIGHT M_HEIGHT

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
#ifdef SUPPORT_FILEFORMAT_JPG
    if (strstr(path, ".png") == NULL && strstr(path, ".jpg") == NULL &&
        strstr(path, ".jpeg") == NULL) {
      printf("ignoring file: %s\n", path);
      return cursor;
    }
#else
    if (strstr(path, ".png") == NULL) {
      printf("ignoring file: %s\n", path);
      return cursor;
    }
#endif // SUPPORT_FILEFORMAT_JPG
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
#ifdef SUPPORT_FILEFORMAT_JPG
    if (strstr(ep->d_name, ".png") == NULL &&
        strstr(ep->d_name, ".jpg") == NULL &&
        strstr(ep->d_name, ".jpeg") == NULL) {
      printf("ignoring file from dir: %s\n", path);
      continue;
    }
#else
    if (strstr(ep->d_name, ".png") == NULL) {
      printf("ignoring file from dir: %s\n", path);
      continue;
    }
#endif // SUPPORT_FILEFORMAT_JPG
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

void help(char *prog_name) {
  printf("NAME\n\t%s - image viewer\n", prog_name);
  printf("\nDESCRIPTION\n\t%s is a simple image viewer written in C using "
         "raylib\n",
         prog_name);
  printf("\nSYNOPSIS\n\t%s <dir|file> ...\n", prog_name);
  printf("\nOPTIONS\n");
  printf("\t%s\n\t\tprint this help\n", "-h, -help, --help, help");
  printf("\nUSAGE\n");
  printf("\t* q\n\t\tclose the application\n");
  printf("\t* LEFT_MOUSE_BUTTON\n\t\tgo to previous image\n");
  printf("\t* MIDDLE_MOUSE_BUTTON\n\t\tdrag current image\n");
  printf("\t* scroll MIDDLE_MOUSE_BUTTON\n\t\tzoom in/out current image\n");
  printf("\t* RIGHT_MOUSE_BUTTON\n\t\tgo to next image\n");
  printf("\nNOTES\n");
  printf("\t* JPG are supported only when raylib is configure and rebuilt with "
         "the jpg option enabled; see README\n");
  printf("\nEXAMPLES\n");
  printf("\t* %s help\n", prog_name);
  printf("\t* %s $HOME/Pictures\n", prog_name);
  printf("\t* %s $HOME/Pictures/example.png\n", prog_name);
  printf(
      "\t* %s $HOME/Pictures/example.jpg # requires jpg support; see README\n",
      prog_name);
  printf("\nAUTHOR\n");
  printf("\tMeelis Utt (meelis.utt@gmail.com)\n");
}

int main(int argc, char **argv) {

  char *prog_name = shift(&argc, &argv);

  CDLLNode *cdll = NULL;
  for (; argc;) {
    char *arg = shift(&argc, &argv);
    if (strcmp("-h", arg) == 0 || strcmp("--help", arg) == 0 ||
        strcmp("help", arg) == 0) {
      help(prog_name);
      goto exit;
    }

    cdll = get_files(cdll, arg);
  }
  cdll_list(cdll);
  viewer(prog_name, cdll);
exit:
  cdll_nodes_free(cdll);
}