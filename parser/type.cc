#include "pop_intface.h"

#include "type.h"
//#include <string.h>
//#include <stdio.h>
#include "parser.h"
//#include <assert.h>

char DataType::stdType[MAXSTDTYPES][32] = {
    "bool",           "char",         "signed char",   "unsigned char", "short",        "short int",
    "unsigned short", "signed short", "int",           "unsigned",      "unsigned int", "signed int",
    "long",           "signed long",  "unsigned long", "long int",      "long long",    "long double",
    "float",          "double",       "void",          "std::string",   "std::string",  "string"};

int DataType::counter = 0;

DataType::DataType(char* tname) {
    isStandard = false;
    isparclass = false;

    if (tname != nullptr) {
        char str[1024];
        char str1[1024];
        char sep[] = " \n\t\r";

        snprintf(str, sizeof(str), "%s", tname);
        char* tmp = strtok(str, sep);
        *str1 = 0;
        while (tmp != nullptr) {
            strcat(str1, tmp);
            tmp = strtok(nullptr, sep);
            if (tmp != nullptr) {
                strcat(str1, " ");
            }
        }
        name = popc_strdup(str1);

        for (auto& elem : stdType)
            if (strcmp(name, elem) == 0) {
                isStandard = true;
                break;
            }
    } else {
        name = nullptr;
    }
    mark = false;
}

DataType::~DataType() {
    if (name != nullptr) {
        free(name);
    }
}

int DataType::CanMarshal() {
    return isStandard;
}

void DataType::Marshal(char* varname, char* bufname, char* sizehelper, std::string& output) {
    if (strcmp(GetName(), "void") != 0) {
        char paramname[256];

        if (!FindVarName(varname, paramname, sizeof(paramname))) {
            snprintf(paramname, sizeof(paramname), "unkown");
        }

        char tmpcode[1024];

        const char* sz = (sizehelper == nullptr) ? "1" : sizehelper;

        snprintf(tmpcode, sizeof(tmpcode), "%s.Push(\"%s\",\"%s\", %s);\n", bufname, paramname, GetName(), sz);
        output += tmpcode;

        snprintf(tmpcode, sizeof(tmpcode), "%s.Pack(&%s, %s);\n", bufname, varname, sz);
        output += tmpcode;

        snprintf(tmpcode, sizeof(tmpcode), "%s.Pop();\n", bufname);
        output += tmpcode;
    }
}

void DataType::DeMarshal(char* varname, char* bufname, char* sizehelper, std::string& output) {
    if (strcmp(GetName(), "void") != 0) {
        char paramname[256];

        if (!FindVarName(varname, paramname, sizeof(paramname))) {
            snprintf(paramname, sizeof(paramname), "unkown");
        }

        char tmpcode[1024];

        const char* sz = (sizehelper == nullptr) ? "1" : sizehelper;
        snprintf(tmpcode, sizeof(tmpcode), "%s.Push(\"%s\",\"%s\", %s);\n", bufname, paramname, GetName(), sz);
        output += tmpcode;

        snprintf(tmpcode, sizeof(tmpcode), "%s.UnPack(&%s,%s);\n", bufname, varname, sz);
        output += tmpcode;

        snprintf(tmpcode, sizeof(tmpcode), "%s.Pop();\n", bufname);
        output += tmpcode;
    }
}

bool DataType::GetDeclaration(const char* varname, char* output) {
    if (name == nullptr) {
        return false;
    }
    if (varname != nullptr) {
        sprintf(output, "%s %s", name, varname);
    } else {
        sprintf(output, "%s", name);
    }
    return true;
}

bool DataType::GetCastType(char* output) {
    return GetDeclaration(nullptr, output);
}

void DataType::GetExpandType(char* output, size_t buffer_length) {
    if (name == nullptr) {
        *output = 0;
    } else {
        snprintf(output, buffer_length, "%s", name);
    }
}

char* DataType::GetName() {
    return name;
}

void DataType::SetName(const char* tname) {
    if (name != nullptr) {
        free(name);
    }
    if (tname == nullptr) {
        name = nullptr;
    } else {
        name = popc_strdup(tname);
        isStandard = false;
        for (auto& elem : stdType)
            if (strcmp(name, elem) == 0) {
                isStandard = true;
                break;
            }
    }
}

int DataType::IsPointer() {
    return 0;
}

bool DataType::IsArray() {
    return false;
}

bool DataType::IsPrototype() {
    return false;
}

bool DataType::IsStandardType() {
    return isStandard;
}

bool DataType::IsParClass() {
    return isparclass;
}

bool DataType::Same(DataType* other) {
    char str1[1024];
    char str2[1024];
    GetExpandType(str1, sizeof(str1));
    other->GetExpandType(str2, sizeof(str2));
    return (strcmp(str1, str2) == 0);
}

bool DataType::Same(char* tname) {
    char str1[1024];
    GetExpandType(str1, sizeof(str1));
    return (strcmp(str1, tname) == 0);
}

DataType* DataType::GetBaseType() {
    return nullptr;
}

void DataType::Mark(bool val) {
    mark = val;
}

bool DataType::IsMarked() {
    return mark;
}

void DataType::SetOwnerFile(CodeFile* owner) {
    codefile = owner;
}

CodeFile* DataType::GetOwnerFile() {
    return codefile;
}

bool DataType::FindVarName(const char* var, char name[256], size_t buffer_length) {
    if (var == nullptr) {
        return false;
    }

    char tmp[1024];
    char* curname = nullptr;
    snprintf(tmp, sizeof(tmp), "%s", var);
    char* tok = curname = strtok(tmp, " .->(*)<>[]");
    while (tok != nullptr) {
        curname = tok;
        tok = strtok(nullptr, " .->(*)<>[]");
    }
    if (curname == nullptr) {
        return false;
    }
    snprintf(name, buffer_length, "%s", curname);
    return true;
}
