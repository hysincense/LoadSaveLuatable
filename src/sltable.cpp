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

#include <assert.h>
#include "lua.hpp"

#include "sltable.h"
#ifdef _WIN32
#define snprintf _snprintf
#endif
Table::Table(){ index = 1; }
Table::~Table()
{
	std::map<SaveValue*, SaveValue*>::iterator   it = mapkeyvaule.begin();
	for (; it != mapkeyvaule.end(); ++it)
	{
		SaveValue * tmp = it->first;
		SaveValue * tmp2 = it->second;
		delete tmp;
		delete tmp2;
	}
	mapkeyvaule.clear();
}

int Table::LoadLuaTable(FILE*fptr)
{
	SaveValue*key = new SaveValue();
	int error = key->Load(fptr);
	while (key->type != LUA_TNONE && error != 1)
	{
		SaveValue*value = new SaveValue();
		error = value->Load(fptr);
		mapkeyvaule[key] = value;
		key = new SaveValue();
		if (error == 1)
			break;
		error = key->Load(fptr);
	}
	delete key;
	if (error == 1)
	{
		return 1;
	}
	return 0;
}
void Table::CreateTable(lua_State *L)
{
	lua_newtable(L);
	std::map<SaveValue*, SaveValue*>::iterator   it = mapkeyvaule.begin();
	for (; it != mapkeyvaule.end(); ++it)
	{
		SaveValue * tmp = it->first;
		tmp->PushLuaValue(L);
		SaveValue * tmp2 = it->second;
		tmp2->PushLuaValue(L);
		lua_settable(L, -3);
	}
}
//------------------------SaveValue
SaveValue::~SaveValue()
{
	if (type == LUA_TTABLE)
	{
		Table * tmp = (Table*)value.table;
		delete tmp;
		value.table = NULL;
	}
	else if (type == LUA_TSTRING)
	{
		char * tmp = value.str;
		delete tmp;
		value.str = NULL;
	}
}
int SaveValue::Load(FILE*fptr)
{
	char ttype;
	fread(&ttype, 1, 1, fptr);
	type = ttype;
	switch (type){
	case LUA_TBOOLEAN:
	{		
		fread(&value.bvalue, 1, 1, fptr);
	}
	break;
	case LUA_TNUMBER:
	{
		char *buf = (char *)&(value.number);
		fread(&buf[7], 1, 1, fptr);
		fread(&buf[6], 1, 1, fptr);
		fread(&buf[5], 1, 1, fptr);
		fread(&buf[4], 1, 1, fptr);
		fread(&buf[3], 1, 1, fptr);
		fread(&buf[2], 1, 1, fptr);
		fread(&buf[1], 1, 1, fptr);
		fread(&buf[0], 1, 1, fptr);
	}
	break;
	case LUA_TSTRING:
	{
		int size = 0;
		fread(&size, 4, 1, fptr);
		if (size > 32732)
		{
			return 1; 
		}
		value.str = new char[size + 2];
		memset(value.str, 0, (size + 2)*sizeof(char));
		fread(value.str, size, 1, fptr);
	}
	break;
	case LUA_TTABLE:
	{
		value.table = new Table();
		return ((Table*)value.table)->LoadLuaTable(fptr);
	}
	break;
	case LUA_TNIL:

		break;
	case LUA_TNONE:
	case 255:
		type = LUA_TNONE;
		break;
	default:
		return 1; 
		break;
	}
	return 0;
}
void SaveValue::PushLuaValue(lua_State *L)
{
	switch (type) {
	case LUA_TBOOLEAN:
		lua_pushboolean(L, value.bvalue == 1);
		break;
	case LUA_TNUMBER:
		lua_pushnumber(L, value.number);
		break;
	case LUA_TSTRING:
		lua_pushstring(L, value.str);
		break;
	case LUA_TTABLE:
		((Table*)value.table)->CreateTable(L);
		break;
	case LUA_TNIL:  break;
		lua_pushnil(L);
	default:
		assert(0 && "invalid type !!!!");
		break;
	}
}

int LoadTable(lua_State *L)
{
	int n = lua_gettop(L);
	size_t len;
	const char* TableName = luaL_checklstring(L, 1, &len);
	
	char filenameSave[1024];
	snprintf(filenameSave, 1024, "%s", TableName);
	
	FILE*fptr = fopen(filenameSave,"rb");
	if (!fptr)
	{
		return 0;
	}
	int c = fgetc(fptr);
	Table*tTable = new Table();
	int error = tTable->LoadLuaTable(fptr);	
	
	fclose(fptr);
	
	if(error == 1)
	{
		lua_pushnil(L);
	}
	else
	{
		tTable->CreateTable(L);
	}
	delete tTable;
	return 1;
} 
 
static void SaveLuaValue(FILE*fptr, lua_State *L, int stackIdx);
static void SaveLuaTable(FILE*fptr, lua_State *L, int stackIdx) {
	assert(stackIdx != -1 && "stack index must not be -1");
	assert(lua_istable(L, stackIdx) && "must be lua table");
	 

	int len = (int)lua_objlen(L, stackIdx);
	lua_pushnil(L);
	while (lua_next(L, stackIdx) != 0) {
		if (lua_isnumber(L, -2)) {
			lua_Number idx = lua_tonumber(L, -2);
			char Ttype = LUA_TNUMBER;
			fwrite(&Ttype, 1, 1, fptr);
			double tmp = idx;
			char *buf = (char *)&tmp;
			fwrite(&buf[7], 1, 1, fptr);
			fwrite(&buf[6], 1, 1, fptr);
			fwrite(&buf[5], 1, 1, fptr);
			fwrite(&buf[4], 1, 1, fptr);
			fwrite(&buf[3], 1, 1, fptr);
			fwrite(&buf[2], 1, 1, fptr);
			fwrite(&buf[1], 1, 1, fptr);
			fwrite(&buf[0], 1, 1, fptr);
		}
		else
		{
			const char *key = luaL_checkstring(L, -2);
			const char * tstr = key;
			int size = strlen(tstr);			
			char Stype = LUA_TSTRING;
			fwrite(&Stype, 1, 1, fptr);
			fwrite(&size, 4, 1, fptr);
			fwrite(tstr, size, 1, fptr);
		}		
		SaveLuaValue(fptr, L, lua_gettop(L));
		lua_pop(L, 1);
	}
	char tmp = -1;
	fwrite(&tmp, 1, 1, fptr);
}
static void SaveLuaValue(FILE*fptr,  lua_State *L, int stackIdx) {
	switch (lua_type(L, stackIdx)) {
	case LUA_TBOOLEAN:
	{
		char b = lua_toboolean(L, stackIdx) == 1 ? 1 : 0;
		char Btype = LUA_TBOOLEAN;
		fwrite(&Btype, 1, 1, fptr);
		fwrite(&b, 1, 1, fptr);
	}
		break;
	case LUA_TNUMBER:
	{
		double tmp = lua_tonumber(L, stackIdx);
		char *buf = (char *)&tmp;
		char Ntype = LUA_TNUMBER;
		fwrite(&Ntype, 1, 1, fptr);

		fwrite(&buf[7], 1, 1, fptr);
		fwrite(&buf[6], 1, 1, fptr);
		fwrite(&buf[5], 1, 1, fptr);
		fwrite(&buf[4], 1, 1, fptr);
		fwrite(&buf[3], 1, 1, fptr);
		fwrite(&buf[2], 1, 1, fptr);
		fwrite(&buf[1], 1, 1, fptr);
		fwrite(&buf[0], 1, 1, fptr);
	}
		break;
	case LUA_TSTRING:
	{
		const char * tstr = lua_tostring(L, stackIdx);
		int size = strlen(tstr);
		char Stype = LUA_TSTRING;
		fwrite(&Stype, 1, 1, fptr);

		fwrite(&size, 4, 1, fptr);
		fwrite(tstr, size, 1, fptr);
	}
		break;
	case LUA_TTABLE:
	{
		char Ttype = LUA_TTABLE;
		fwrite(&Ttype, 1, 1, fptr);
		SaveLuaTable(fptr, L, stackIdx);
	}
		break;
	case LUA_TNIL:  break;
	{
		char NILtype = LUA_TNIL;
		fwrite(&NILtype, 1, 1, fptr);
	}
	default:
		assert(0 && "invalid lua type !!!!");
		break;
	}
}

int SaveTable(lua_State *L)
{
	int n = lua_gettop(L);
	size_t len;
	const char* TableName = luaL_checklstring(L, 1, &len);

	char filenameSave[1024];
	snprintf(filenameSave, 1024, "%s", TableName);
	
	FILE*fptr = fopen(filenameSave,"wb");
	if (!fptr)
	{
		return 0;
	}

	int argLen = n - 1;
	SaveLuaValue(fptr, L, 0 + 2);
	fclose(fptr);
	return 0;
}

int DeleteFile(lua_State *L)
{
    int n = lua_gettop(L);
	size_t len;
	const char* FileName = luaL_checklstring(L, 1, &len);

	char filenameSave[1024];
	snprintf(filenameSave, 1024, "%s", FileName);

    int ret = remove(filenameSave);
    lua_pushinteger(L, ret);
	return 1;
}

//------------------------------------------------------------------------------------------

static const struct luaL_Reg emptyfuns[] = {
	{ NULL, NULL }
};

static const struct luaL_Reg funs[] = {
	{ "load", LoadTable },
	{ "save", SaveTable },
	{ "delfile", DeleteFile },
	{ NULL, NULL }
};

int ext_lua_sltable(lua_State *L)
{
	luaL_newmetatable(L, SLTABLE_NAMESPACE);
	luaL_register(L, NULL, emptyfuns);
	luaL_register(L, SLTABLE_NAMESPACE, funs);
	lua_pop(L, 2);
	return 0;
}