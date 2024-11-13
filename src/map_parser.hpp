#ifndef MAP_PARSER_HPP
#define MAP_PARSER_HPP

#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>
#include "map.hpp"

std::map<long long int, Node*> nodes;
std::vector<Way*> ways;

// LOADS .osmania files (best map format ever :D)
void loadMap(std::string file) {
  std::ifstream mapFile(file);
  if (!mapFile) {
    std::cout << "Couldn't open the map file!" << std::endl;
    return;
  }

  std::string line;
  bool lookingForNodes = false, lookingForWays = false;
  while (std::getline(mapFile, line)) {
    if (!lookingForNodes) {
      if (line.find("NODES") != std::string::npos) {
        lookingForNodes = true;
        continue;
      }
    }

    if (lookingForNodes) {
      if (line.find("WAYS") != std::string::npos) {
        lookingForNodes = false;
        lookingForWays = true;
        continue;
      }
      int idPos = 0;
      std::string id = "";
      char currentChar = line[idPos];
      do {
        id += currentChar;
        idPos++;
        currentChar = line[idPos];
      } while (currentChar != ' ');

      idPos++;
      currentChar = line[idPos];
      std::string xString = "";
      do {
        xString += currentChar;
        idPos++;
        currentChar = line[idPos];
      } while (currentChar != ' ');

      idPos++;
      currentChar = line[idPos];
      std::string yString = "";
      do {
        yString += currentChar;
        idPos++;
        if (idPos == line.length()) break;
        currentChar = line[idPos];
      } while (currentChar != ' ');

      nodes[std::stoll(id)] = loadNodeXY(std::stoll(id), std::stof(xString), std::stof(yString));
      //nodes.insert(std::make_pair(std::stoll(id), loadNodeXY(std::stoll(id), std::stof(xString), std::stof(yString))));
    }

    if (lookingForWays) {
      int idPos = 0;
      std::string id = "";
      char currentChar = line[idPos];
      do {
        id += currentChar;
        idPos++;
        if (idPos == line.length()) break;
        currentChar = line[idPos];
      } while (currentChar != ' ');

      Way* way = loadWay(std::stoll(id));
      
      Node* lastNode = nullptr;
      std::string currentId = "";
      for (int i = idPos + 1; i < line.length(); i++) {
        if (line[i] == ' ') {
          Node* currentNode = nodes[std::stoll(currentId)];
          if (currentNode != nullptr && lastNode != nullptr) {
            currentNode->neighbors.push_back(lastNode);
            lastNode->neighbors.push_back(currentNode);
          }
          if (currentNode != nullptr) {
            way->nodes.push_back(currentNode);
            lastNode = currentNode;
          }
          currentId = "";
          continue;
        } else currentId += line[i];
      }
      lastNode = nullptr;

      ways.push_back(way);
    }
  }

}



// LOADS .osm files

std::map<long long int, Node*> loadNodes(std::string file) {
  std::map<long long int, Node*> nodes;
  
  std::ifstream mapFile(file);

  if (!mapFile) {
    std::cout << "Couldn't open the map file!";
    return nodes;
  }

  std::string line;
  int count = 0;
  while (std::getline(mapFile, line)) {
    count++;
    if (line.find("<node ") != std::string::npos) {
      int idPos = line.find("id") + 4;
      std::string id = "";
      char currentChar = line[idPos];
      do {
        id += currentChar;
        idPos++;
        currentChar = line[idPos];
      } while (currentChar != '"');

      
      int latPos = line.find("lat") + 5;
      std::string lat = "";
      currentChar = line[latPos];
      do {
        lat += currentChar;
        latPos++;
        currentChar = line[latPos];
      } while (currentChar != '"');
      
      int lonPos = line.find("lon") + 5;
      std::string lon = "";
      currentChar = line[lonPos];
      do {
        lon += currentChar;
        lonPos++;
        currentChar = line[lonPos];
      } while (currentChar != '"');

      nodes[std::stoll(id)] = (loadNode(std::stoll(id), std::stof(lat), std::stof(lon)));
    }
  }

  mapFile.close();

  return nodes;
}


std::vector<Way*> loadWays(std::string file) {
  std::vector<Way*> ways;

  std::ifstream mapFile(file);

  if (!mapFile) {
    std::cout << "Couldn't open the map file!";
    return ways;
  }

  std::string line;
  int count = 0;

  Way* currentWay = nullptr;
  bool isHighway = false;
  Node* lastNode = nullptr;
  while (std::getline(mapFile, line)) {
    count++;
    if (currentWay == nullptr && line.find("<way ") != std::string::npos) {
      int idPos = line.find("id") + 4;
      std::string id = "";
      char currentChar = line[idPos];
      do {
        id += currentChar;
        idPos++;
        currentChar = line[idPos];
      } while (currentChar != '"');

      currentWay = loadWay(std::stoll(id));
    }

    if (currentWay != nullptr) {
      auto p = line.find("<nd ref");

      if (p == std::string::npos) {
        if (line.find("</way>") != std::string::npos) {
          lastNode = nullptr;
          if (isHighway) ways.push_back(currentWay);

          else delete currentWay;
          currentWay = nullptr;
          isHighway = false;
        } else if (line.find("highway") != std::string::npos) {
          isHighway = true;
        }
        continue;
      }

      int nodePos = p + 9;

      std::string ref = "";
      char currentChar = line[nodePos];
      do {
        ref += currentChar;
        nodePos++;
        currentChar = line[nodePos];
      } while (currentChar != '"');

      Node* currentNode = nodes[std::stoll(ref)];
      if (currentNode != nullptr && lastNode != nullptr) {
        currentNode->neighbors.push_back(lastNode);
        lastNode->neighbors.push_back(currentNode);
      }
      currentWay->nodes.push_back(currentNode);
      lastNode = currentNode;
    }
  }

  mapFile.close();
  
  return ways;
}
#endif
