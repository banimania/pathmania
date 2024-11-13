#ifndef MAP_CONVERTER_HPP
#define MAP_CONVERTER_HPP

#include "map_parser.hpp"
#include <fstream>
#include <utility>

// converts osm maps to my mercator format with precomputer xy coords. worst map format ever???
void optimizeMap() {
  std::ofstream optimizedFile("res/valencia2.osm");
  
  optimizedFile << "NODES\n";
  for (std::pair<long long int, Node*> pair : nodes) {
    if (pair.second == nullptr) continue;

    optimizedFile << pair.second->id << " " << std::to_string(pair.second->x) << " " << std::to_string(pair.second->y) << "\n";
  }
  
  optimizedFile << "WAYS\n";
  for (int i = 0; i < ways.size(); i++) {
    Way* way = ways[i];
    if (way == nullptr) continue;

    std::string idRefs = "";
    for (int j = 0; j < way->nodes.size(); j++) {
      if (way->nodes[j] == nullptr) continue;
      idRefs += std::to_string(way->nodes[j]->id) + " ";
    }

    optimizedFile << i << " " << idRefs << "\n";
  }
  optimizedFile.close();
}

#endif
