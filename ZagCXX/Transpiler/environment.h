#pragma once

#include <string>
#include <vector>

#include <ZagIR/Ast/node.h>

#include "object.h"

using namespace ZagIR;
namespace fs = std::filesystem;

namespace ZagCXX {

class Object;
class ObjectType;
class ObjectCOperation;
class ObjectProtoType;

class Scope {
public:
  void Print();
  void Delete();
  std::unordered_map<std::string, Object*> data;
};

class Environment {
public:
  Environment();
  ~Environment();

  void DumpEnvironment();

  void PushScope();
  void PopScope();
  int ScopeCount();

  void AddPackageToScope(ZagIR::Package *package);
  void AddSubPackageToScope(ZagIR::Package *package, std::string subpackage);

  void AddToRoot(std::string, Object*);
  void AddToScope(std::string, Object*);
  void AddToReserved(std::string, Object*);

  void AddInclude(std::string);
  void AddInclude(fs::path);
  void AddFileDep(ZagIR::Package *package, std::string);

  std::string GetIncludes();
  std::string GetCXXArgs();
  void GetLibNames(std::vector<std::string> *);

  bool ExistsInScope(std::string);
  bool Exists(std::string);
  bool ExistsInReserved(std::string);
  
  bool IsConcretedFrom(ObjectType*, std::string); // Type from left is concreted from right

  Object *Fetch(std::string);
  Object *FetchRoot(std::string);
  ObjectProtoType *FetchProtoType(std::string);
  
  ObjectType *FetchType(std::string);
  ObjectType *FetchType(Node*);

  Object* FetchReserved(std::string);
  ObjectCOperation *FetchOperation(ObjectType*, ObjectType*);
private:
  std::string recurseGetType(Node* );
  ObjectType* FetchExistingType(std::string);

  std::vector<std::string> includes;
  std::vector<std::string> absoluteIncludes;
  std::vector<std::string> packageNames;

  std::string cxxargs;

  std::string includeGlob;
  std::vector<std::string> fileDeps;
  std::vector<Scope> environment;
  Scope reserved;
};

}; // namespace ZagCXX
