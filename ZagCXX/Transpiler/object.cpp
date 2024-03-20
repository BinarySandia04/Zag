#include "object.h"

#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>

#include <ZagIR/Logs/logs.h>

#include "termcolor/termcolor.hpp"

using namespace ZagIR;
using namespace ZagCXX;

Object *ZagCXX::GetObjectFromBinding(Binding *b) {
  CType *ctype = dynamic_cast<CType *>(b);
  CFunction *cfunction = dynamic_cast<CFunction *>(b);
  Conversion *conversion = dynamic_cast<Conversion *>(b);
  COperation *coperation = dynamic_cast<COperation *>(b);

  if (ctype != nullptr) {
    return new ObjectProtoType(ctype);
  } else if (cfunction != nullptr) {
    return new ObjectCFunction(cfunction);
  } else if (conversion != nullptr) {
    return new ObjectConversion(conversion);
  } else if (coperation != nullptr) {
    return new ObjectCOperation(coperation);
  }
  return new ObjectEmpty();
}

void Object::Print(int space) { std::cout << "[Object]" << std::endl; }

void ObjectEmpty::Print(int space) {
  std::cout << "[ObjectEmpty]" << std::endl;
}

Object* ObjectEmpty::Clone(){
  return new ObjectEmpty();
}

void ObjectVariable::Print(int space) {
  std::cout << "[ObjectVariable]" << std::endl;
}

Object* ObjectVariable::Clone(){
  return new ObjectVariable(type, name);
}

ObjectVariable::ObjectVariable(ObjectType *type, std::string name) {
  this->type = type;
  this->name = name;
}

void ObjectVariable::SetType(ObjectType *type) { this->type = type; }

ObjectType *ObjectVariable::GetType() { return type; }

std::string ObjectVariable::Transpile() { return "_v_" + this->name; }

ObjectContainer::ObjectContainer() {}

ObjectContainer::ObjectContainer(std::vector<Binding *> *bindings) {
  for (int i = 0; i < bindings->size(); i++) {
    AddBinding((*bindings)[i]);
  }
}

ObjectContainer::~ObjectContainer() {
  for (auto &p : containerData) {
    delete p.second;
  }
}

void ObjectContainer::Print(int space) {
  std::cout << std::string(space, ' ') << "[ObjectContainer]" << std::endl;
  for (auto &p : containerData) {
    std::cout << std::string(space + 3, ' ') << termcolor::yellow << p.first
              << termcolor::reset << ": ";
    if (p.second != nullptr)
      p.second->Print(space + 3);
    else
      std::cout << "nullptr" << std::endl;
  }
}

Object* ObjectContainer::Clone(){
  ObjectContainer* newContainer = new ObjectContainer();
  for(auto &p : containerData){
    newContainer->AddObject(p.second->Clone(), p.first);
  }
  return newContainer;
}

void ObjectContainer::AddObject(Object *obj, std::string path) {
  // FirstPart: io
  // SecondPart: hola.tal.print
  std::string firstPart = "", secondPart = "";

  // Populate firstPart and secondPart
  for (int i = 0; i < path.size(); i++) {
    if (path[i] != '.') {
      firstPart += path[i];
    } else {
      i++;
      while (i < path.size()) {
        secondPart += path[i];
        i++;
      }
    }
  }

  if (secondPart == "") {
    containerData[firstPart] = obj;
  } else {
    ObjectContainer *subContainer;
    if (containerData.find(firstPart) != containerData.end()) {
      // Comprovem que es container
      subContainer = dynamic_cast<ObjectContainer *>(containerData[firstPart]);
      if (subContainer == nullptr) {
        std::cout << firstPart << "|" << secondPart << std::endl;
        Logs::Error("Tried to add object to an existing container");
        return;
      }
    } else {
      subContainer = new ObjectContainer();
      containerData[firstPart] = subContainer;
    }

    subContainer->AddObject(obj, secondPart);
  }
}

void ObjectContainer::AddBinding(ZagIR::Binding *b) {
  AddObject(GetObjectFromBinding(b), b->name);
}

Object *ObjectContainer::GetObject(std::string key) {
  return containerData[key];
}

void ObjectContainer::Merge(ObjectContainer* other){
  for(auto &p : other->containerData){
    AddObject(p.second->Clone(), p.first);
  }
}

void ObjectFunction::Print(int space) {
  std::cout << "[ObjectFunction]" << std::endl;
}

Object* ObjectFunction::Clone(){
  ObjectFunction* copy = new ObjectFunction();
  copy->functionArgs = functionArgs;
  copy->returnType = returnType;
  copy->inheritedType = inheritedType;
}

void ObjectFunction::SetInheritedType(ObjectType *type) {
  this->inheritedType = type;
  for (int i = 0; i < functionArgs.size(); i++) {
    std::string arg = functionArgs[i];
    if (arg.size() == 0)
      continue; // ??? HUH
    if (arg[0] == '$') {
      try {
        int posArg = std::stoi(arg.substr(1, arg.size() - 1)) - 1;
        functionArgs[i] = type->children[posArg]->identifier;
      } catch (std::exception &ex) {
        Logs::Error(ex.what());
      }
    }
  }
}

std::string ObjectFunction::GetName() { return "_f_" + this->identifier; }

bool ObjectFunction::CheckArgs(std::vector<ObjectType *> &args,
                               Environment *env) {
  if (args.size() != functionArgs.size()) {
    return false;
  }
  for (int i = 0; i < args.size(); i++) {
    if (functionArgs[i] == "Any")
      continue;

    ObjectType *argType = env->FetchType(functionArgs[i]);
    if (!args[i]->AbstractedFrom(argType)) {
      Logs::Error(args[i]->identifier + " " + functionArgs[i]);
      return false;
    }
  }
  return true;
}

ObjectCFunction::ObjectCFunction(ZagIR::CFunction *cfunction) {
  this->cFunctionData = cfunction;
  for (int i = 0; i < cfunction->funcArgs.size(); i++)
    this->functionArgs.push_back(cfunction->funcArgs[i]);
  this->returnType = cfunction->retType;
}

void ObjectCFunction::Print(int space) {
  std::cout << "[ObjectCFunction]" << std::endl;
}

Object* ObjectCFunction::Clone(){
  return new ObjectCFunction(cFunctionData);
}

void ObjectCFunction::Use(Environment *t) {
  for (int i = 0; i < cFunctionData->headers.size(); i++) {
    fs::path filePath = fs::path("src") /
                        cFunctionData->package->path.filename() /
                        cFunctionData->headers[i];
    t->AddInclude(filePath);
  }
}

std::string ObjectCFunction::GetName() { return this->cFunctionData->bind; }

void ObjectNativeFunction::Print(int space) {
  std::cout << "[ObjectNativeFunction]" << std::endl;
}

Object* ObjectNativeFunction::Clone(){
  return new ObjectNativeFunction();
}

std::string ObjectNativeFunction::GetName() { return "_f_" + this->identifier; }

ObjectConversion::ObjectConversion(ZagIR::Conversion *conversion) {
  this->conversion = conversion;
}

void ObjectConversion::Print(int space) {
  std::cout << "[ObjectConversion]" << std::endl;
}

Object* ObjectConversion::Clone(){
  return new ObjectConversion(conversion);
}

ObjectProtoType::ObjectProtoType(CType *cTypeInfo) {
  this->cTypeInfo = cTypeInfo;
  this->typeMethods = new ObjectContainer(&(cTypeInfo->children));
}

ObjectProtoType::~ObjectProtoType() { delete typeMethods; }

void ObjectProtoType::Print(int n) {
  std::cout << "[ObjectProtoType]" << std::endl;
}

void ObjectProtoType::Use(Environment *env) {
  // Logs::Debug("Used");
  for (int i = 0; i < cTypeInfo->headers.size(); i++) {
    fs::path filePath = fs::path("src") / cTypeInfo->package->path.filename() /
                        cTypeInfo->headers[i];
    env->AddInclude(filePath);
  }
  for (int i = 0; i < cTypeInfo->include.size(); i++) {
    env->AddInclude(cTypeInfo->include[i]);
  }
}

ObjectType *ObjectProtoType::Construct(std::vector<ObjectType *> args,
                                       Environment *env) {
  // If we construct it we suppose that it doesnt exists
  // We check first that the number of args are the same
  Use(env);

  ObjectType *constructed = new ObjectType();
  constructed->children = args;

  constructed->identifier = cTypeInfo->name;
  constructed->translation = cTypeInfo->parent;
  constructed->constructor = this;
  if (args.size() > 0) {
    constructed->identifier += "<";
    for (int i = 0; i < args.size(); i++) {
      constructed->identifier += args[i]->identifier;
      if (i < args.size() - 1)
        constructed->identifier += ",";
    }
    constructed->identifier += ">";
  }

  env->AddToReserved(constructed->identifier, constructed);
  return constructed;
}

void ObjectType::Print(int space) { std::cout << "[ObjectType]" << std::endl; }

Object* ObjectType::Clone(){
  ObjectType* newType = new ObjectType();
  newType->identifier = identifier;
  newType->translation = translation;
  newType->upgrades_to = upgrades_to;

  for(int i = 0; i < children.size(); i++) newType->children.push_back(dynamic_cast<ObjectType*>(children[i]->Clone()));
  newType->constructor = constructor;
}

Object* ObjectProtoType::Clone(){
  ObjectProtoType* newProtoType = new ObjectProtoType(cTypeInfo);
  newProtoType->typeMethods = dynamic_cast<ObjectContainer*>(typeMethods->Clone());
  return newProtoType;
}

bool ObjectType::Equals(ObjectType *other) { return this == other; }

bool ObjectType::AbstractedFrom(ObjectType *abstract) {
  if (abstract->constructor == constructor) {
    bool res = true;
    for (int i = 0; i < abstract->children.size(); i++)
      res = res && children[i]->AbstractedFrom(abstract->children[i]);
    return res;
  }
  return false;
}

std::string ObjectType::Transpile() {
  std::string t = translation;

  if (children.size() > 0) {
    t += "<";
    for (int i = 0; i < children.size(); i++)
      t += children[i]->Transpile();
    t += ">";
  }
  return t;
}

ObjectCOperation::ObjectCOperation(COperation *operation) {
  this->cOperationData = operation;
}

void ObjectCOperation::Print(int n) {
  std::cout << "[ObjectCOperation]" << std::endl;
}

Object* ObjectCOperation::Clone(){
  return new ObjectCOperation(cOperationData);
}

std::string ObjectCOperation::GetName() { return this->cOperationData->bind; }

void ObjectCOperation::Use(Environment *t) {
  for (int i = 0; i < cOperationData->headers.size(); i++) {
    fs::path filePath = fs::path("src") /
                        cOperationData->package->path.filename() /
                        cOperationData->headers[i];
    t->AddInclude(filePath);
  }
}
