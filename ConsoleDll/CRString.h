#pragma once
#include <string>
#include <vector>

using namespace std;

typedef vector<string> StringVector;
//不能导出使用
void EncryptString(string & src);

void DecryptString(string & src);