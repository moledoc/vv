#include <assert.h>
#include <stdio.h>

#include "raylib.h"
#include "raymath.h"

int main() {
  InitWindow(800, 600, "vv");
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