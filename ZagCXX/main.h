#pragma once

#include <filesystem>
#include <string>

void TranspileFile(std::string);
void Transpile(std::string, std::string);
int Compile(std::filesystem::path, std::filesystem::path, std::string);
void Run(std::filesystem::path);
