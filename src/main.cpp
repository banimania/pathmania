#include <algorithm>
#include <climits>
#include <cstdlib>
#include <queue>
#include <raylib.h>
#include <rlgl.h>
#include <raymath.h>
#include <math.h>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
#include "map.hpp"
#include "map_converter.hpp"
#include "map_parser.hpp"
#if defined(__EMSCRIPTEN__)
  #include <emscripten/emscripten.h>
#endif

const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 720;
const char* WINDOW_NAME = "PathMania";

const int GAME_SCREEN_WIDTH = 1280;
const int GAME_SCREEN_HEIGHT = 720;

Camera2D cam;

Vector2 mouse;

RenderTexture2D target;

long long int inicioId = -1;
long long int finalId = -1;
long long int closestInicioId = -1;
long long int closestFinalId = -1;
bool inicio = false;
bool final = false;
float inicioX = 0;
float inicioY = 0;
float finalX = 0;
float finalY = 0;
float minDistInicio = INT_MAX;
float minDistFinal = INT_MAX;

std::vector<std::pair<Vector2, Vector2>> visitedLines;
std::vector<Node*> solution;
int calls = 40;

std::priority_queue<std::pair<float, Node*>, std::vector<std::pair<float, Node*>>, std::greater<>> pq;
std::unordered_map<long long int, float> dist;
std::unordered_map<long long int, Node*> prev;

void aStar() {
  if (dist.empty()) {
    for (std::pair<long long int, Node*> pair : nodes) {
      dist[pair.first] = std::numeric_limits<float>::infinity();
    }
    dist[inicioId] = 0;
    float h_inicio = Vector2Distance(
      {nodes[inicioId]->x, nodes[inicioId]->y}, 
      {nodes[finalId]->x, nodes[finalId]->y}
    );
    pq.push({h_inicio, nodes[inicioId]});
  }

  if (!pq.empty()) {
    std::pair<float, Node*> currentPair = pq.top();
    float current_f = currentPair.first;
    Node* current = currentPair.second;
    pq.pop();

    if (current->id == finalId) {
      pq = std::priority_queue<std::pair<float, Node*>, std::vector<std::pair<float, Node*>>, std::greater<>>();
      for (Node* at = nodes[finalId]; at != nullptr; at = prev[at->id]) {
        solution.push_back(at);
      }
      std::reverse(solution.begin(), solution.end());
      return;
    }

    for (Node* neighbor : current->neighbors) {
      visitedLines.push_back(std::make_pair(Vector2{neighbor->x, neighbor->y}, Vector2{current->x, current->y}));
      float g_new = dist[current->id] + Vector2Distance({current->x, current->y}, {neighbor->x, neighbor->y});
      float h_new = Vector2Distance(
        {neighbor->x, neighbor->y}, 
        {nodes[finalId]->x, nodes[finalId]->y}
      );

      if (g_new < dist[neighbor->id]) {
        dist[neighbor->id] = g_new;
        prev[neighbor->id] = current;
        pq.push({g_new + h_new, neighbor});
      }
    }
  }
}

void dijkstra() {
  if (dist.empty()) {
    for (std::pair<long long int, Node*> pair : nodes) {
      dist[pair.first] = std::numeric_limits<float>::infinity();
    }
    dist[inicioId] = 0;
    pq.push({0, nodes[inicioId]});
  }

  if (!pq.empty()) {
    std::pair<float, Node*> currentPair = pq.top();
    float current_dist = currentPair.first;
    Node* current = currentPair.second;
    pq.pop();

    if (current->id == finalId) {
      pq = std::priority_queue<std::pair<float, Node*>, std::vector<std::pair<float, Node*>>, std::greater<>>();
      for (Node* at = nodes[finalId]; at != nullptr; at = prev[at->id]) {
        solution.push_back(at);
      }
      std::reverse(solution.begin(), solution.end());
      return;
    }

    for (Node* neighbor : current->neighbors) {
      visitedLines.push_back(std::make_pair(Vector2{neighbor->x, neighbor->y}, Vector2{current->x, current->y}));
      float new_dist = current_dist + Vector2Distance({current->x, current->y}, {neighbor->x, neighbor->y});

      if (new_dist < dist[neighbor->id]) {
        dist[neighbor->id] = new_dist;
        prev[neighbor->id] = current;
        pq.push({new_dist, neighbor});
      }
    }
  }
}

bool loading = false;

void mainLoop() {
  float scale = fmin((float) GetScreenWidth() / GAME_SCREEN_WIDTH, (float) GetScreenHeight() / GAME_SCREEN_HEIGHT);

  mouse = GetMousePosition();
  mouse.x = (mouse.x - (GetScreenWidth() - (GAME_SCREEN_WIDTH * scale)) * 0.5f) / scale;
  mouse.y = (mouse.y - (GetScreenHeight() - (GAME_SCREEN_HEIGHT * scale)) * 0.5f) / scale;
  mouse = Vector2Clamp(mouse, (Vector2){ 0, 0 }, (Vector2){ (float) GAME_SCREEN_WIDTH, (float) GAME_SCREEN_HEIGHT });

  /*if (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE)) {
    Vector2 delta = GetMouseDelta();
    delta = {delta.x / cam.zoom, delta.y / cam.zoom};
    cam.target = Vector2Subtract(cam.target, delta);
  }

  float wheelDelta = GetMouseWheelMove();
  if (wheelDelta != 0) {
    Vector2 mouseWorldPos = GetScreenToWorld2D(GetMousePosition(), cam);

    cam.offset = GetMousePosition();

    cam.target = mouseWorldPos;

    float scaleFactor = 1.0f + (0.25f * fabsf(wheelDelta));
    if (wheelDelta < 0) scaleFactor = 1.0f / scaleFactor;
    cam.zoom = Clamp(cam.zoom * scaleFactor, 0.0125f, 64.0f);
  }*/

  BeginTextureMode(target);
  ClearBackground(BLACK);
  BeginMode2D(cam);

  Rectangle viewRect = { cam.target.x - cam.offset.x / cam.zoom, cam.target.y - cam.offset.y / cam.zoom, GetScreenWidth() / cam.zoom, GetScreenHeight() / cam.zoom };
 

  if (loading) {
    loadMap("res/valencia2.osm");
  }

  if (nodes.empty() || ways.empty()) {
    DrawText("Loading map...", 200, 300, 100, WHITE);
    loading = true;
  } else loading = false;

  if (IsKeyDown(KEY_SPACE) && inicioId != -1 && finalId != -1) {
    //for (int i = 0; i < calls; i++) dijkstra();
    for (int i = 0; i < calls; i++) aStar();
    calls++;
  }

  Vector2 mouseWorldPos = GetScreenToWorld2D(mouse, cam);
  if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
    inicio = true;
    inicioX = mouseWorldPos.x;
    inicioY = mouseWorldPos.y;
    inicioId = -1;
  }
  
  if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
    final = true;
    finalX = mouseWorldPos.x;
    finalY = mouseWorldPos.y;
    finalId = -1;
  }

  rlDrawRenderBatchActive();
  rlSetLineWidth(1.0);
  for (long long int i = 0; i < ways.size(); i++) {
    if (ways[i] == nullptr || ways[i]->nodes.empty()) continue;
    for (long long int j = 1; j < ways[i]->nodes.size(); j++) {
      if (ways[i]->nodes[j] == nullptr || ways[i]->nodes[j - 1] == nullptr) continue;

      Vector2 startPos = { ways[i]->nodes[j - 1]->x, ways[i]->nodes[j - 1]->y };
      Vector2 endPos = { ways[i]->nodes[j]->x, ways[i]->nodes[j]->y };
      
      if (CheckCollisionPointRec(startPos, viewRect) || CheckCollisionPointRec(endPos, viewRect)) {
        DrawLine(startPos.x, startPos.y, endPos.x, endPos.y, WHITE);

        if (inicio && inicioId == -1) {
          int thisDistance = Vector2Distance({startPos.x, startPos.y}, {inicioX, inicioY});
          if (minDistInicio > thisDistance) {
            minDistInicio = thisDistance;
            closestInicioId = ways[i]->nodes[j]->id;
          }
        }
        if (final && finalId == -1) {
          int thisDistance = Vector2Distance({startPos.x, startPos.y}, {finalX, finalY});
          if (minDistFinal > thisDistance) {
            minDistFinal = thisDistance;
            closestFinalId = ways[i]->nodes[j]->id;
          }
        }
      }
    }
  }
  rlDrawRenderBatchActive();
  rlSetLineWidth(1.0);

  if (inicio && inicioId == -1) {
    inicioId = closestInicioId;
    closestInicioId = -1;
    minDistInicio = INT_MAX;
    solution.clear();
    visitedLines.clear();
    pq = std::priority_queue<std::pair<float, Node*>, std::vector<std::pair<float, Node*>>, std::greater<>>();
    dist.clear();
    prev.clear();
    calls = 40;
  }
  if (final && finalId == -1) {
    finalId = closestFinalId;
    closestFinalId = -1;
    minDistFinal = INT_MAX;
    solution.clear();
    visitedLines.clear();
    pq = std::priority_queue<std::pair<float, Node*>, std::vector<std::pair<float, Node*>>, std::greater<>>();
    dist.clear();
    prev.clear();
    calls = 40;
  }

  if (inicio && inicioId != -1) {
    DrawCircle(nodes[inicioId]->x, nodes[inicioId]->y, 5, {255, 0, 0, 200});
  }
  if (final && finalId != -1) {
    DrawCircle(nodes[finalId]->x, nodes[finalId]->y, 5, {255, 0, 0, 200});
  }

  rlDrawRenderBatchActive();
  rlSetLineWidth(1.5);
  for (std::pair<Vector2, Vector2> line : visitedLines) {
    DrawLineV(line.first, line.second, BLUE);
  }
  rlDrawRenderBatchActive();
  rlSetLineWidth(1.5);

  rlDrawRenderBatchActive();
  rlSetLineWidth(10.0);
  for (int i = 1; i < solution.size(); i++) {
    DrawLine(solution[i]->x, solution[i]->y, solution[i - 1]->x, solution[i - 1]->y, GREEN);
  }
  rlDrawRenderBatchActive();
  rlSetLineWidth(1);

  DrawText("Right click -> Start", 600, -140, 10.0, WHITE);
  DrawText("Left click -> Finish", 600, -130, 10.0, WHITE);
  DrawText("Spacebar -> Solve", 600, -120, 10.0, WHITE);

  EndMode2D();
  EndTextureMode();

  BeginDrawing();
  ClearBackground(BLACK);
  DrawTexturePro(target.texture, (Rectangle){ 0.0f, 0.0f, (float) target.texture.width, (float) -target.texture.height },
                         (Rectangle){ (GetScreenWidth() - ((float) GAME_SCREEN_WIDTH * scale)) * 0.5f, (GetScreenHeight() - ((float) GAME_SCREEN_HEIGHT * scale)) * 0.5f,
                         (float) GAME_SCREEN_WIDTH * scale, (float) GAME_SCREEN_HEIGHT * scale }, (Vector2) { 0, 0 }, 0.0f, WHITE);
  EndDrawing();
}

int main() {
  SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
  InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_NAME);

  target = LoadRenderTexture(GAME_SCREEN_WIDTH, GAME_SCREEN_HEIGHT);
  SetTextureFilter(target.texture, TEXTURE_FILTER_BILINEAR);

  SetTargetFPS(60);

  cam = {  };
  cam.zoom = 1.25;
  cam.target.x = 527;
  cam.target.y = 265;
  cam.offset.x = 776;
  cam.offset.y = 520;

  // Load the map
  /*nodes = loadNodes(file);
  ways = loadWays(file);
  optimizeMap();*/

#if defined(__EMSCRIPTEN__)
  emscripten_set_main_loop(mainLoop, 240, 1);
#else
  while (!WindowShouldClose()) {
    mainLoop();
  }
#endif
  UnloadRenderTexture(target);
  CloseWindow();
  return 0;
}
