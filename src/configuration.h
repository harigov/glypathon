#pragma once

#include <iostream>
#include <map>
#include <thread>

class Configuration
{
 public:
  static Configuration& Instance();
  ~Configuration();

  void Load(const std::string& filename);
  void Stop();

  int ReadInt(const std::string& name);
  float ReadFloat(const std::string& name);
  double ReadDouble(const std::string& name);
  bool ReadBool(const std::string& name);
  std::string ReadString(const std::string& name);

 private:
  Configuration();
  static void Reader(Configuration* instance, const std::string& filename);
  void ReadFile(const std::string& filename);

  std::thread reader_;
  std::map<std::string, std::string> variables_[2];
  int idx_;
  bool quit_;
};
