#ifndef MAP_HPP
#define MAP_HPP

#include <cmath>
#include <vector>

const float EARTH_RADIUS = 6371.0;
const float LAT_ORIGIN = 39.4841406;
const float LON_ORIGIN = -0.3841131;

struct Way;

struct Node {
  long long int id;
  float x, y;

  std::vector<Node*> neighbors;
  
  Node(long long int id, float x, float y) : id(id), x(x), y(y) {}
};

struct Way {
  long long int id;
  std::vector<Node*> nodes;

  Way(long long int id) : id(id) {}
};

Node* loadNode(long long int id, float lat, float lon) {
  double latRad = lat * M_PI / 180.0;
  double lonRad = lon * M_PI / 180.0;

  double latOriginRad = LAT_ORIGIN * M_PI / 180.0;
  double lonOriginRad = LON_ORIGIN * M_PI / 180.0;

  double dLat = latRad - latOriginRad;
  double dLon = lonRad - lonOriginRad;

  double xMeters = dLon * EARTH_RADIUS * cos(latOriginRad) * 1000.0;
  double yMeters = dLat * EARTH_RADIUS * 1000.0;

  float x = xMeters / 10.0;
  float y = -yMeters / 10.0;

  return new Node(id, x, y);
}

Node* loadNodeXY(long long int id, float x, float y) {
  return new Node(id, x, y);
}

Way* loadWay(long long int id) {
  return new Way(id);
}

void addNodeToWay(Way* way, Node* node) {
  way->nodes.push_back(node);
}

#endif
