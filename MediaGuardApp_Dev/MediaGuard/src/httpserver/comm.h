#pragma once

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>
#include <map>
#ifdef _WIN32
//
#include <windows.h>
#endif

#ifndef MAKE64
#define MAKE64(_high32,_low32)  (((UINT64)_high32)<<32) | ((UINT64)_low32)
#endif

//int64_t getFileSize(const char* fileName);
size_t getFileCtx(const char* fileName, char* ctx, size_t ctxsize);
int GetExecPath(char* p, int size);

size_t split(const std::string& s, const std::string& seperator, std::vector<std::string>& strlist);
void parse_web_param(const std::string& str, std::map<std::string, std::string>& mp_kv);
