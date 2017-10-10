 
// The MIT License (MIT)

// Copyright 2010-2017 hysincense
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
#ifndef empireofsparta_SaveLoadTable_h
#define empireofsparta_SaveLoadTable_h

#include <map>
#include <string>
#include <vector>
#include "lua.h"
#define SLTABLE_NAMESPACE "sltable"
 
int SaveTable(lua_State *L);
int LoadTable(lua_State *L);
int DeleteFile(lua_State *L);

typedef union {
	double number;
	char*str;
	char bvalue;
	void*table;
} cusSavValue;
class SaveValue
{
public:
	int type;
	cusSavValue value;
	int Load(FILE*fptr);
	void PushLuaValue(lua_State *L);
	~SaveValue();
};

class Table
{
public:
	int index;
	std::map<SaveValue*, SaveValue*> mapkeyvaule;
	Table();
	int LoadLuaTable(FILE*fptr);
 
	void CreateTable(lua_State *L);
	~Table();
};
#ifdef __cplusplus
extern "C" {
#endif

	int ext_lua_sltable(lua_State *L);
#ifdef __cplusplus
}
#endif

#endif
