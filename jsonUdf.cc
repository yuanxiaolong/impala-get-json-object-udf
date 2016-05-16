// Copyright 2015 Steven Han
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// Author : Steven Han <nazgul33 at gmail.com>
// Rapid json is obtainable at
//
// current status:
// - input should be a String
// - output is always a String
// - llvm udf works fine. .so udf wasn't tested
// - selector is the same with hive get_json_object, except that * operator is not supported


// we have to define this macro before <inttypes.h> is included
// so compile on linux works
#define __STDC_FORMAT_MACROS

#include <stdlib.h>
#include "jsonUdf.h"
#include <impala_udf/udf-debug.h>

#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

#include <string>
#include <vector>
#include <iostream>

#include <stdio.h>
#include <inttypes.h>

StringVal JsonGetObject(FunctionContext *context, const StringVal & jsonVal, const StringVal & selectorVal) {

try {
    if (jsonVal.is_null) return StringVal::null();
    if (selectorVal.is_null) return StringVal::null();

    std::string json((const char*)jsonVal.ptr, jsonVal.len);
    std::string selector((const char*)selectorVal.ptr, selectorVal.len);

    rapidjson::Document d;
    d.Parse(json.c_str());
    if (d.HasParseError()) {
        return StringVal::null();
    }

    const char * pSel = selector.c_str();
    rapidjson::Value *currentVal = NULL;
    int i;

    std::string token;

//#define selectValByToken(tok) { \
//    rapidjson::Value& va = *currentVal;  \
//    if (va.HasMember(tok.c_str())) { \
//        currentVal = &(va[tok.c_str()]); \
//    } else { \
//        return StringVal::null(); \
//    } \
//}

    if (pSel[0] != '$') {
//        context->SetError("seletor should start with $");
        return StringVal::null();
    }


    currentVal = &d;
    for (i=1; i<selector.size(); i++) {
        switch (pSel[i]) {
        case '$':
//            context->SetError("$ can only be placed at start");
            return StringVal::null();
        case '.':
//            if (token.size() != 0)
//                selectValByToken(token);
            token.clear();
            break;
        default:
            token += pSel[i];
            break;
        }
    }


    if (token.size() != 0) {
//        rapidjson::Value &v = d[token.c_str()];
        rapidjson::Value& va = *currentVal;
        if (va.HasMember(token.c_str())) {
//        if (json.find(token) != std::string::npos){
            currentVal = &(va[token.c_str()]);
//            if (currentVal == NULL){
//                return StringVal::null();
//            }
        } else {
            return StringVal::null();
        }
        token.clear();
    }
//
//    if (currentVal->IsNull()) {
//        return StringVal::null();
//    }
//    if (currentVal->IsBool()) {
//        bool val = currentVal->GetBool();
//        if (val) {
//            StringVal result((uint8_t*)"true", 4);
//            return result;
//        } else {
//            StringVal result((uint8_t*)"false", 5);
//            return result;
//        }
//    }
//    if (currentVal->IsNumber()) {
//        StringVal result(context, 32);
//        int len;
//        if (currentVal->IsInt64()) {
//            // covers int, int64, uint
//            len = snprintf((char*)result.ptr, 32, "%" PRId64, currentVal->GetInt64());
//        } else if (currentVal->IsUint64()) {
//            // covers uint64
//            len = snprintf((char*)result.ptr, 32, "%" PRIu64, currentVal->GetUint64());
//        } else {
//            // covers double
//            len = snprintf((char*)result.ptr, 32, "%f", currentVal->GetDouble());
//        }
//        result.len = len;
//        return result;
//    }
    if (currentVal->IsString()) {
        StringVal result(context, currentVal->GetStringLength());
        memcpy(result.ptr, currentVal->GetString(), currentVal->GetStringLength());
//        memset(currentVal,0, currentVal->GetStringLength());
        return result;
    }



    return StringVal::null();
}catch(...){
    printf("now catch it %s , ignore","error");
    return StringVal::null();
}
}
