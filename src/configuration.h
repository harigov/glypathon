#pragma once

#include <iostream>
#include <map>
#include <mutex>
#include <thread>

class Configuration
{
 public:
  static Configuration& Instance();
  ~Configuration();

  int ReadInt(const std::string name);
  float ReadFloat(const std::string name);
  bool ReadBool(const std::string name);
  std::string ReadString(const std::string name);

 private:
  Configuration();
  static void Reader(Configuration* instance, const std::string& filename);
  void ReadFile(const std::string& filename);

  std::thread reader_;
  std::mutex mutex_;
  std::map<std::string, std::string> variables_;
};
