/*
 * DictionaryImp.cpp
 *
 *  Created on: Oct 23, 2013
 *      Author: dzhou
 */

#include "DictionaryImp.h"
#include "Constant.h"
#include "Exceptions.h"
#include "Guid.h"
#include "Types.h"
#include "Util.h"

#include <algorithm>
#include <cstring>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

using std::unordered_map;
using std::string;
namespace dolphindb {

void stringU8VectorReader(const ConstantSP& value, int start, int count, U8* output){
	char* tmp;
#ifdef BIT32
	char** buf=(char**)(output+count/2);
#else
	char** buf=(char**)output;
#endif
	char** pbuf;
	std::size_t len;
	pbuf=value->getStringConst(start,count,buf);
	for(int i=0;i<count;++i){
		len=strlen(pbuf[i])+1;
		tmp=new char[len];
		memcpy(tmp,pbuf[i],len);
		output[i].pointer=tmp;
	}
}

void stringU8ScalarReader(const ConstantSP& value, U8& output){
	const std::string & str=value->getStringRef();
	size_t len=str.length()+1;
	output.pointer=new char[len];
	memcpy(output.pointer,str.c_str(),len);
}

void stringU8VectorWriter(U8* input, const ConstantSP& output, int start, int count){
#ifdef BIT32
	char** buf=(char**)input;
	for(int i=1;i<count;++i)
		buf[i]=input[i].pointer;
	output->setString(start,count,buf);
#else
	output->setString(start,count,(char**)input);
#endif
}

void stringU8ScalarWriter(const U8& value, const ConstantSP& output){
	output->setString(value.pointer);
}

void doubleU8VectorReader(const ConstantSP& value, int start, int count, U8* output){
	value->getDouble(start,count,(double*)output);
}

void doubleU8ScalarReader(const ConstantSP& value, U8& output){
	output.doubleVal=value->getDouble();
}

void doubleU8VectorWriter(U8* input, const ConstantSP& output, int start, int count){
	output->setDouble(start,count,(double*)input);
}

void doubleU8ScalarWriter(const U8& value, const ConstantSP& output){
	output->setDouble(value.doubleVal);
}

void floatU8VectorReader(const ConstantSP& value, int start, int count, U8* output){
	auto* buf=(float*)(output+(count/2));
	const float* pbuf=value->getFloatConst(start,count,buf);
	for(int i=0;i<count;i++)
		output[i].floatVal=pbuf[i];
}

void floatU8ScalarReader(const ConstantSP& value, U8& output){
	output.floatVal=value->getFloat();
}

void floatU8VectorWriter(U8* input, const ConstantSP& output, int start, int count){
	auto* buf=(float*)input;
	float* pbuf=output->getFloatBuffer(start,count,buf);
	for(int i=0;i<count;++i)
		pbuf[i]=input[i].floatVal;
	output->setFloat(start,count,pbuf);
}

void floatU8ScalarWriter(const U8& value, const ConstantSP& output){
	output->setFloat(value.floatVal);
}

void longU8VectorReader(const ConstantSP& value, int start, int count, U8* output){
	value->getLong(start,count,(long long*)output);
}

void longU8ScalarReader(const ConstantSP& value, U8& output){
	output.longVal=value->getLong();
}

void longU8VectorWriter(U8* input, const ConstantSP& output, int start, int count){
	output->setLong(start,count,(long long*)input);
}

void longU8ScalarWriter(const U8& value, const ConstantSP& output){
	output->setLong(value.longVal);
}

void intU8VectorReader(const ConstantSP& value, int start, int count, U8* output){
	int* buf=(int*)(output+(count/2));
	const int* pbuf=value->getIntConst(start,count,buf);
	for(int i=0;i<count;i++)
		output[i].intVal=pbuf[i];
}

void intU8ScalarReader(const ConstantSP& value, U8& output){
	output.intVal=value->getInt();
}

void intU8VectorWriter(U8* input, const ConstantSP& output, int start, int count){
	int* buf=(int*)input;
	int* pbuf=output->getIntBuffer(start,count,buf);
	for(int i=0;i<count;++i)
		pbuf[i]=input[i].intVal;
	output->setInt(start,count,pbuf);
}

void intU8ScalarWriter(const U8& value, const ConstantSP& output){
	output->setInt(value.intVal);
}

void shortU8VectorReader(const ConstantSP& value, int start, int count, U8* output){
	auto* buf=(short*)(output+(count/2));
	const short* pbuf=value->getShortConst(start,count,buf);
	for(int i=0;i<count;i++)
		output[i].shortVal=pbuf[i];
}

void shortU8ScalarReader(const ConstantSP& value, U8& output){
	output.shortVal=value->getShort();
}

void shortU8VectorWriter(U8* input, const ConstantSP& output, int start, int count){
	auto* buf=(short*)input;
	short* pbuf=output->getShortBuffer(start,count,buf);
	for(int i=0;i<count;++i)
		pbuf[i]=input[i].shortVal;
	output->setShort(start,count,pbuf);
}

void shortU8ScalarWriter(const U8& value, const ConstantSP& output){
	output->setShort(value.shortVal);
}

void charU8VectorReader(const ConstantSP& value, int start, int count, U8* output){
	char* buf=(char*)(output+(count/2));
	const char* pbuf=value->getCharConst(start,count,buf);
	for(int i=0;i<count;i++)
		output[i].charVal=pbuf[i];
}

void charU8ScalarReader(const ConstantSP& value, U8& output){
	output.charVal=value->getChar();
}

void charU8VectorWriter(U8* input, const ConstantSP& output, int start, int count){
	char* buf=(char*)input;
	char* pbuf=output->getCharBuffer(start,count,buf);
	for(int i=0;i<count;++i)
		pbuf[i]=input[i].charVal;
	output->setChar(start,count,pbuf);
}

void charU8ScalarWriter(const U8& value, const ConstantSP& output){
	output->setChar(value.charVal);
}

void boolU8VectorReader(const ConstantSP& value, int start, int count, U8* output){
	char* buf=(char*)(output+(count/2));
	const char* pbuf=value->getBoolConst(start,count,buf);
	for(int i=0;i<count;i++)
		output[i].charVal=pbuf[i];
}

void boolU8ScalarReader(const ConstantSP& value, U8& output){
	output.charVal=value->getBool();
}

void boolU8VectorWriter(U8* input, const ConstantSP& output, int start, int count){
	char* buf=(char*)input;
	char* pbuf=output->getBoolBuffer(start,count,buf);
	for(int i=0;i<count;++i)
		pbuf[i]=input[i].charVal;
	output->setBool(start,count,pbuf);
}

void boolU8ScalarWriter(const U8& value, const ConstantSP& output){
	output->setBool(value.charVal);
}


void AbstractDictionary::init(){
    switch (internalType_)
    {
    case DT_BOOL:
        vreader_=&boolU8VectorReader;
        sreader_=&boolU8ScalarReader;
        vwriter_=&boolU8VectorWriter;
        swriter_=&boolU8ScalarWriter;
        nullVal_.charVal=Constant::void_->getBool();
        break;
    case DT_CHAR:
        vreader_=&charU8VectorReader;
        sreader_=&charU8ScalarReader;
        vwriter_=&charU8VectorWriter;
        swriter_=&charU8ScalarWriter;
        nullVal_.charVal=Constant::void_->getChar();
        break;
    case DT_SHORT:
        vreader_=&shortU8VectorReader;
        sreader_=&shortU8ScalarReader;
        vwriter_=&shortU8VectorWriter;
        swriter_=&shortU8ScalarWriter;
        nullVal_.shortVal=Constant::void_->getShort();
        break;
    case DT_INT:
        vreader_=&intU8VectorReader;
        sreader_=&intU8ScalarReader;
        vwriter_=&intU8VectorWriter;
        swriter_=&intU8ScalarWriter;
        nullVal_.intVal=Constant::void_->getInt();
        break;
    case DT_LONG:
        vreader_=&longU8VectorReader;
        sreader_=&longU8ScalarReader;
        vwriter_=&longU8VectorWriter;
        swriter_=&longU8ScalarWriter;
        nullVal_.longVal=Constant::void_->getLong();
        break;
    case DT_FLOAT:
        vreader_=&floatU8VectorReader;
        sreader_=&floatU8ScalarReader;
        vwriter_=&floatU8VectorWriter;
        swriter_=&floatU8ScalarWriter;
        nullVal_.floatVal=Constant::void_->getFloat();
        break;
    case DT_DOUBLE:
        vreader_=&doubleU8VectorReader;
        sreader_=&doubleU8ScalarReader;
        vwriter_=&doubleU8VectorWriter;
        swriter_=&doubleU8ScalarWriter;
        nullVal_.doubleVal=Constant::void_->getDouble();
        break;
    case DT_STRING:
    case DT_BLOB:
        vreader_=&stringU8VectorReader;
        sreader_=&stringU8ScalarReader;
        vwriter_=&stringU8VectorWriter;
        swriter_=&stringU8ScalarWriter;
        nullVal_.pointer=(char*)(Constant::EMPTY.c_str());
        break;
    case DT_ANY:
        break;
    default:
        throw RuntimeException("Not Support create this internalType dictionary " + Util::getDataTypeString(internalType_));
    }
}

ConstantSP AbstractDictionary::createValues(const ConstantSP& key) const{
	if(key->isScalar())
		return Util::createConstant(type_);
	return Util::createVector(type_,key->size());
}

IntDictionary::IntDictionary(const unordered_map<int,U8>& dict, DATA_TYPE keyType,DATA_TYPE type)
	:AbstractDictionary(keyType,type),dict_(dict){
	if(type_!=DT_STRING)
		return;
	auto it=dict_.begin();
	auto end=dict_.end();
	size_t len;
	char* tmp;
	while(it!=end){
		len=strlen(it->second.pointer)+1;
		tmp=new char[len];
		memcpy(tmp,it->second.pointer,len);
		it->second.pointer=tmp;
		++it;
	}
}

IntDictionary::~IntDictionary(){
	if(type_ != DT_STRING && type_ != DT_BLOB)
		return;
	auto it=dict_.begin();
	auto end=dict_.end();
	while(it!=end){
		delete[] it->second.pointer;
		++it;
	}
}

bool IntDictionary::remove(const ConstantSP& key){
	if(key->isScalar())
		dict_.erase(key->getInt());
	else{
		int len=key->size();
		const int bufSize=Util::BUF_SIZE;
		int buf[bufSize];
		const int* pbuf;
		int counts;
		int start = 0;
		while(start < len){
			counts = ((std::min))(len - start, bufSize);
			pbuf = key->getIntConst(start, counts, buf);
			for(int i=0; i<counts; ++i)
				dict_.erase(pbuf[i]);
			start += counts;
		}
	}
	return true;
}

bool IntDictionary::set(const ConstantSP& key, const ConstantSP& value){
	ConstantSP newKey;
	if(keyCategory_==TEMPORAL && key->getType()!=keyType_)
		return false;
	newKey=key;

	if(newKey->isScalar()){
		U8 tmp;
		(*sreader_)(value,tmp);
		dict_[newKey->getInt()]=tmp;
		return true;
	}
	int len=newKey->size();
	if(len!=value->size())
		return false;

	if(dict_.empty())
		dict_.reserve((int)(len*1.33));
	const int bufSize = Util::BUF_SIZE;
	int buf[bufSize];
	const int *pbuf;
	U8 tmp[bufSize];
	int start = 0;
	int counts;
	std::size_t dictSize = dict_.size();
	while (start < len) {
		counts = ((std::min))(len - start, bufSize);
		pbuf = newKey->getIntConst(start, counts, buf);
		(*vreader_)(value, start, counts, tmp);
		if (type_ == DT_STRING) {
			for (int i = 0; i < counts; ++i) {
				U8 &cur = dict_[pbuf[i]];
				if (dict_.size() == dictSize)
					delete[] cur.pointer;
				else
					++dictSize;
				cur = tmp[i];
			}
		} else {
			for (int i = 0; i < counts; ++i)
				dict_[pbuf[i]] = tmp[i];
		}
		start += counts;
	}
	return true;
}

ConstantSP IntDictionary::getMember(const ConstantSP& key) const{
	ConstantSP newKey;
	if(keyCategory_==TEMPORAL && key->getType()!=keyType_)
		throw IncompatibleTypeException(keyType_, key->getType());
	newKey=key;

	ConstantSP result=createValues(newKey);
	if(newKey->isScalar()){
		auto it=dict_.find(newKey->getInt());
		if(it==dict_.end())
			(*swriter_)(nullVal_,result);
		else
			(*swriter_)(it->second,result);
	}
	else{
		int len=newKey->size();
		const int bufSize=Util::BUF_SIZE;
		int buf[bufSize];
		const int* pbuf;
		U8 value[bufSize];
		int start=0;
		int counts;
		unordered_map<int,U8>::const_iterator it;
		auto end=dict_.end();
		while(start<len){
			counts=((std::min))(len-start,bufSize);
			pbuf=newKey->getIntConst(start,counts,buf);
			for(int i=0;i<counts;++i){
				it=dict_.find(pbuf[i]);
				value[i]=it==end?nullVal_:it->second;
			}
			(*vwriter_)(value,result,start,counts);
			start+=counts;
		}
		result->setNullFlag(result->hasNull());
	}
	return result;
}

void IntDictionary::contain(const ConstantSP& target, const ConstantSP& resultSP) const{
	if(target->isScalar()){
		resultSP->setBool(static_cast<char>(dict_.find(target->getInt())!=dict_.end()));
	}
	else{
		int len=target->size();
		const int bufSize=Util::BUF_SIZE;
		char ret[bufSize];
		char* pret;
		int buf[bufSize];
		const int* pbuf;
		int start=0;
		int counts;

		unordered_map<int,U8>::const_iterator it;
		auto end=dict_.end();
		while(start<len){
			counts=((std::min))(len-start,bufSize);
			pbuf=target->getIntConst(start,counts,buf);
			pret=resultSP->getBoolBuffer(start,counts,ret);
			for(int i=0;i<counts;++i)
				pret[i]=static_cast<char>(dict_.find(pbuf[i])!=end);
			resultSP->setBool(start,counts,pret);
			start+=counts;
		}
	}
}

ConstantSP IntDictionary::keys() const {
	auto it=dict_.begin();
	int len=size();
	ConstantSP resultSP=Util::createVector(keyType_,len);
	int start=0;
	int counts;
	const int bufSize=Util::BUF_SIZE;
	int buf[bufSize];
	int* pbuf;

	while(start<len){
		counts=((std::min))(len-start,bufSize);
		pbuf=resultSP->getIntBuffer(start,counts,buf);
		for(int i=0;i<counts;++i){
			pbuf[i]=it->first;
			++it;
		}
		resultSP->setInt(start,counts,pbuf);
		start+=counts;
	}
	return resultSP;
}

ConstantSP IntDictionary::values() const {
	auto it=dict_.begin();
	int len=size();
	ConstantSP resultSP= Util::createVector(type_,len);

	int start=0;
	int counts;
	const int bufSize=Util::BUF_SIZE;
	U8 buf[bufSize];

	while(start<len){
		counts=((std::min))(len-start,bufSize);
		for(int i=0;i<counts;++i){
			buf[i]=it->second;
			++it;
		}
		(*vwriter_)(buf,resultSP,start,counts);
		start+=counts;
	}
	return resultSP;
}

std::string IntDictionary::getString() const {
	string content;
	int len=((std::min))(Util::DISPLAY_ROWS,(int)dict_.size());
	int counts=0;
	auto it=dict_.begin();
	ConstantSP key=Util::createConstant(keyType_);
	ConstantSP value=Util::createConstant(type_);
	while(counts<len){
		key->setInt(it->first);
		content.append(key->getString());
		content.append("->");
		(*swriter_)(it->second,value);
		content.append(value->getString());
		content.append(1,'\n');
		++counts;
		++it;
	}
	if(len<(int)dict_.size())
		content.append("...\n");
	return content;
}

long long IntDictionary::getAllocatedMemory() const {
	long long bytes=sizeof(IntDictionary)+(size()*12);
	if(getType()==DT_STRING){
		auto it=dict_.begin();
		auto end=dict_.end();
		while(it!=end){
			bytes += strlen(it->second.pointer);
			++it;
		}
	}
	return bytes;
}

CharDictionary::CharDictionary(const unordered_map<char,U8>& dict, DATA_TYPE keyType,DATA_TYPE type)
	:AbstractDictionary(keyType,type),dict_(dict){
	if(type_!=DT_STRING)
		return;
	auto it=dict_.begin();
	auto end=dict_.end();
	size_t len;
	char* tmp;
	while(it!=end){
		len=strlen(it->second.pointer)+1;
		tmp=new char[len];
		memcpy(tmp,it->second.pointer,len);
		it->second.pointer=tmp;
		++it;
	}
}

CharDictionary::~CharDictionary(){
	if(type_ != DT_STRING && type_ != DT_BLOB)
		return;
	auto it=dict_.begin();
	auto end=dict_.end();
	while(it!=end){
		delete[] it->second.pointer;
		++it;
	}
}

bool CharDictionary::remove(const ConstantSP& key){
	if(key->isScalar())
		dict_.erase(key->getChar());
	else{
		int len=key->size();
		const int bufSize=Util::BUF_SIZE;
		char buf[bufSize];
		const char* pbuf;
		int counts;
		int start = 0;
		while(start < len){
			counts = ((std::min))(len - start, bufSize);
			pbuf = key->getCharConst(start, counts, buf);
			for(int i=0; i<counts; ++i)
				dict_.erase(pbuf[i]);
			start += counts;
		}
	}
	return true;
}

bool CharDictionary::set(const ConstantSP& key, const ConstantSP& value){
	if(key->isScalar()){
		U8 tmp;
		(*sreader_)(value,tmp);
		dict_[key->getChar()]=tmp;
		return true;
	}
	int len = key->size();
	if (len != value->size())
		return false;

	if (dict_.empty())
		dict_.reserve((int)(len * 1.33));
	const int bufSize = Util::BUF_SIZE;
	char buf[bufSize];
	const char *pbuf;
	U8 tmp[bufSize];
	int start = 0;
	int counts;
	std::size_t dictSize = dict_.size();
	while (start < len) {
		counts = ((std::min))(len - start, bufSize);
		pbuf = key->getCharConst(start, counts, buf);
		(*vreader_)(value, start, counts, tmp);
		if (type_ == DT_STRING) {
			for (int i = 0; i < counts; ++i) {
				U8 &cur = dict_[pbuf[i]];
				if (dict_.size() == dictSize)
					delete[] cur.pointer;
				else
					++dictSize;
				cur = tmp[i];
			}
		} else {
			for (int i = 0; i < counts; ++i)
				dict_[pbuf[i]] = tmp[i];
		}
		start += counts;
	}
	return true;
}

ConstantSP CharDictionary::getMember(const ConstantSP& key) const {
	ConstantSP result=createValues(key);
	if(key->isScalar()){
		auto it=dict_.find(key->getChar());
		if(it==dict_.end())
			(*swriter_)(nullVal_,result);
		else
			(*swriter_)(it->second,result);
	}
	else{
		int len=key->size();
		const int bufSize=Util::BUF_SIZE;
		char buf[bufSize];
		const char* pbuf;
		U8 value[bufSize];
		int start=0;
		int counts;
		unordered_map<char,U8>::const_iterator it;
		auto end=dict_.end();
		while(start<len){
			counts=((std::min))(len-start,bufSize);
			pbuf=key->getCharConst(start,counts,buf);
			for(int i=0;i<counts;++i){
				it=dict_.find(pbuf[i]);
				value[i]=it==end?nullVal_:it->second;
			}
			(*vwriter_)(value,result,start,counts);
			start+=counts;
		}
		result->setNullFlag(result->hasNull());
	}
	return result;
}

void CharDictionary::contain(const ConstantSP& target, const ConstantSP& resultSP) const{
	if(target->isScalar()){
		resultSP->setBool(static_cast<char>(dict_.find(target->getChar())!=dict_.end()));
	}
	else{
		int len=target->size();
		const int bufSize=Util::BUF_SIZE;
		char ret[bufSize];
		char* pret;
		char buf[bufSize];
		const char* pbuf;
		int start=0;
		int counts;

		unordered_map<char,U8>::const_iterator it;
		auto end=dict_.end();
		while(start<len){
			counts=((std::min))(len-start,bufSize);
			pbuf=target->getCharConst(start,counts,buf);
			pret=resultSP->getBoolBuffer(start,counts,ret);
			for(int i=0;i<counts;++i)
				pret[i]=static_cast<char>(dict_.find(pbuf[i])!=end);
			resultSP->setBool(start,counts,pret);
			start+=counts;
		}
	}
}

ConstantSP CharDictionary::keys() const {
	auto it=dict_.begin();
	int len=size();
	ConstantSP resultSP=Util::createVector(keyType_,len);
	int start=0;
	int counts;
	const int bufSize=Util::BUF_SIZE;
	char buf[bufSize];
	char* pbuf;

	while(start<len){
		counts=((std::min))(len-start,bufSize);
		pbuf=resultSP->getCharBuffer(start,counts,buf);
		for(int i=0;i<counts;++i){
			pbuf[i]=it->first;
			++it;
		}
		resultSP->setChar(start,counts,pbuf);
		start+=counts;
	}
	return resultSP;
}

ConstantSP CharDictionary::values() const {
	auto it=dict_.begin();
	int len=size();
	ConstantSP resultSP= Util::createVector(type_,len);

	int start=0;
	int counts;
	const int bufSize=Util::BUF_SIZE;
	U8 buf[bufSize];

	while(start<len){
		counts=((std::min))(len-start,bufSize);
		for(int i=0;i<counts;++i){
			buf[i]=it->second;
			++it;
		}
		(*vwriter_)(buf,resultSP,start,counts);
		start+=counts;
	}
	return resultSP;
}

std::string CharDictionary::getString() const{
	string content;
	int len=((std::min))(Util::DISPLAY_ROWS,(int)dict_.size());
	int counts=0;
	auto it=dict_.begin();
	ConstantSP key=Util::createConstant(keyType_);
	ConstantSP value=Util::createConstant(type_);
	while(counts<len){
		key->setChar(it->first);
		content.append(key->getString());
		content.append("->");
		(*swriter_)(it->second,value);
		content.append(value->getString());
		content.append(1,'\n');
		++counts;
		++it;
	}
	if(len<(int)dict_.size())
		content.append("...\n");
	return content;
}

long long CharDictionary::getAllocatedMemory() const {
	long long bytes=sizeof(CharDictionary)+(size()*9);
	if(getType()==DT_STRING){
		auto it=dict_.begin();
		auto end=dict_.end();
		while(it!=end){
			bytes += strlen(it->second.pointer);
			++it;
		}
	}
	return bytes;
}

ShortDictionary::ShortDictionary(const unordered_map<short,U8>& dict, DATA_TYPE keyType, DATA_TYPE type)
	:AbstractDictionary(keyType,type),dict_(dict){
	if(type_!=DT_STRING)
		return;
	auto it=dict_.begin();
	auto end=dict_.end();
	size_t len;
	char* tmp;
	while(it!=end){
		len=strlen(it->second.pointer)+1;
		tmp=new char[len];
		memcpy(tmp,it->second.pointer,len);
		it->second.pointer=tmp;
		++it;
	}
}

ShortDictionary::~ShortDictionary(){
	if(type_ != DT_STRING && type_ != DT_BLOB)
		return;
	auto it=dict_.begin();
	auto end=dict_.end();
	while(it!=end){
		delete[] it->second.pointer;
		++it;
	}
}

bool ShortDictionary::remove(const ConstantSP& key){
	if(key->isScalar())
		dict_.erase(key->getShort());
	else{
		int len=key->size();
		const int bufSize=Util::BUF_SIZE;
		short buf[bufSize];
		const short* pbuf;
		int counts;
		int start = 0;
		while(start < len){
			counts = ((std::min))(len - start, bufSize);
			pbuf = key->getShortConst(start, counts, buf);
			for(int i=0; i<counts; ++i)
				dict_.erase(pbuf[i]);
			start += counts;
		}
	}
	return true;
}

bool ShortDictionary::set(const ConstantSP& key, const ConstantSP& value){
	if(key->isScalar()){
		U8 tmp;
		(*sreader_)(value,tmp);
		dict_[key->getShort()]=tmp;
		return true;
	}
	int len = key->size();
	if (len != value->size())
		return false;

	if (dict_.empty())
		dict_.reserve((int)(len * 1.33));
	const int bufSize = Util::BUF_SIZE;
	short buf[bufSize];
	const short *pbuf;
	U8 tmp[bufSize];
	int start = 0;
	int counts;
	std::size_t dictSize = dict_.size();
	while (start < len) {
		counts = ((std::min))(len - start, bufSize);
		pbuf = key->getShortConst(start, counts, buf);
		(*vreader_)(value, start, counts, tmp);
		if (type_ == DT_STRING) {
			for (int i = 0; i < counts; ++i) {
				U8 &cur = dict_[pbuf[i]];
				if (dict_.size() == dictSize)
					delete[] cur.pointer;
				else
					++dictSize;
				cur = tmp[i];
			}
		} else {
			for (int i = 0; i < counts; ++i)
				dict_[pbuf[i]] = tmp[i];
		}
		start += counts;
	}
	return true;
}

ConstantSP ShortDictionary::getMember(const ConstantSP& key) const {
	ConstantSP result=createValues(key);
	if(key->isScalar()){
		auto it=dict_.find(key->getShort());
		if(it==dict_.end())
			(*swriter_)(nullVal_,result);
		else
			(*swriter_)(it->second,result);
	}
	else{
		int len=key->size();
		const int bufSize=Util::BUF_SIZE;
		short buf[bufSize];
		const short* pbuf;
		U8 value[bufSize];
		int start=0;
		int counts;
		unordered_map<short,U8>::const_iterator it;
		auto end=dict_.end();
		while(start<len){
			counts=((std::min))(len-start,bufSize);
			pbuf=key->getShortConst(start,counts,buf);
			for(int i=0;i<counts;++i){
				it=dict_.find(pbuf[i]);
				value[i]=it==end?nullVal_:it->second;
			}
			(*vwriter_)(value,result,start,counts);
			start+=counts;
		}
		result->setNullFlag(result->hasNull());
	}
	return result;
}

void ShortDictionary::contain(const ConstantSP& target, const ConstantSP& resultSP) const{
	if(target->isScalar()){
		resultSP->setBool(static_cast<char>(dict_.find(target->getShort())!=dict_.end()));
	}
	else{
		int len=target->size();
		const int bufSize=Util::BUF_SIZE;
		char ret[bufSize];
		char* pret;
		short buf[bufSize];
		const short* pbuf;
		int start=0;
		int counts;

		unordered_map<short,U8>::const_iterator it;
		auto end=dict_.end();
		while(start<len){
			counts=((std::min))(len-start,bufSize);
			pbuf=target->getShortConst(start,counts,buf);
			pret=resultSP->getBoolBuffer(start,counts,ret);
			for(int i=0;i<counts;++i)
				pret[i]=static_cast<char>(dict_.find(pbuf[i])!=end);
			resultSP->setBool(start,counts,pret);
			start+=counts;
		}
	}
}

ConstantSP ShortDictionary::keys() const {
	auto it=dict_.begin();
	int len=size();
	ConstantSP resultSP=Util::createVector(keyType_,len);
	int start=0;
	int counts;
	const int bufSize=Util::BUF_SIZE;
	short buf[bufSize];
	short* pbuf;

	while(start<len){
		counts=((std::min))(len-start,bufSize);
		pbuf=resultSP->getShortBuffer(start,counts,buf);
		for(int i=0;i<counts;++i){
			pbuf[i]=it->first;
			++it;
		}
		resultSP->setShort(start,counts,pbuf);
		start+=counts;
	}
	return resultSP;
}

ConstantSP ShortDictionary::values() const {
	auto it=dict_.begin();
	int len=size();
	ConstantSP resultSP= Util::createVector(type_,len);

	int start=0;
	int counts;
	const int bufSize=Util::BUF_SIZE;
	U8 buf[bufSize];

	while(start<len){
		counts=(std::min)(len-start,bufSize);
		for(int i=0;i<counts;++i){
			buf[i]=it->second;
			++it;
		}
		(*vwriter_)(buf,resultSP,start,counts);
		start+=counts;
	}
	return resultSP;
}

std::string ShortDictionary::getString() const{
	string content;
	int len=(std::min)(Util::DISPLAY_ROWS,(int)dict_.size());
	int counts=0;
	auto it=dict_.begin();
	ConstantSP key=Util::createConstant(keyType_);
	ConstantSP value=Util::createConstant(type_);
	while(counts<len){
		key->setShort(it->first);
		content.append(key->getString());
		content.append("->");
		(*swriter_)(it->second,value);
		content.append(value->getString());
		content.append(1,'\n');
		++counts;
		++it;
	}
	if(len<(int)dict_.size())
		content.append("...\n");
	return content;
}

long long ShortDictionary::getAllocatedMemory() const {
	long long bytes=sizeof(ShortDictionary)+(size()*10);
	if(getType()==DT_STRING){
		auto it=dict_.begin();
		auto end=dict_.end();
		while(it!=end){
			bytes += strlen(it->second.pointer);
			++it;
		}
	}
	return bytes;
}

LongDictionary::LongDictionary(const unordered_map<long long,U8>& dict, DATA_TYPE keyType, DATA_TYPE type)
	:AbstractDictionary(keyType,type),dict_(dict){
	if(type_!=DT_STRING)
		return;
	auto it=dict_.begin();
	auto end=dict_.end();
	size_t len;
	char* tmp;
	while(it!=end){
		len=strlen(it->second.pointer)+1;
		tmp=new char[len];
		memcpy(tmp,it->second.pointer,len);
		it->second.pointer=tmp;
		++it;
	}
}

LongDictionary::~LongDictionary(){
	if(type_ != DT_STRING && type_ != DT_BLOB)
		return;
	auto it=dict_.begin();
	auto end=dict_.end();
	while(it!=end){
		delete[] it->second.pointer;
		++it;
	}
}

bool LongDictionary::remove(const ConstantSP& key){
	if(key->isScalar())
		dict_.erase(key->getLong());
	else{
		int len=key->size();
		const int bufSize=Util::BUF_SIZE;
		long long buf[bufSize];
		const long long* pbuf;
		int counts;
		int start = 0;
		while(start < len){
			counts = (std::min)(len - start, bufSize);
			pbuf = key->getLongConst(start, counts, buf);
			for(int i=0; i<counts; ++i)
				dict_.erase(pbuf[i]);
			start += counts;
		}
	}
	return true;
}

bool LongDictionary::set(const ConstantSP& key, const ConstantSP& value){
	ConstantSP newKey;
	if(keyCategory_==TEMPORAL && key->getType()!=keyType_)
		return false;
	newKey=key;

	if(newKey->isScalar()){
		U8 tmp;
		(*sreader_)(value,tmp);
		dict_[newKey->getLong()]=tmp;
		return true;
	}
	int len = newKey->size();
	if (len != value->size())
		return false;

	if (dict_.empty())
		dict_.reserve((int)(len * 1.33));
	const int bufSize = Util::BUF_SIZE;
	long long buf[bufSize];
	const long long *pbuf;
	U8 tmp[bufSize];
	int start = 0;
	int counts;
	std::size_t dictSize = dict_.size();
	while (start < len)
	{
		counts = (std::min)(len - start, bufSize);
		pbuf = newKey->getLongConst(start, counts, buf);
		(*vreader_)(value, start, counts, tmp);
		if (type_ == DT_STRING) {
			for (int i = 0; i < counts; ++i) {
				U8 &cur = dict_[pbuf[i]];
				if (dict_.size() == dictSize)
					delete[] cur.pointer;
				else
					++dictSize;
				cur = tmp[i];
			}
		} else {
			for (int i = 0; i < counts; ++i)
				dict_[pbuf[i]] = tmp[i];
		}
		start += counts;
	}
	return true;
}

ConstantSP LongDictionary::getMember(const ConstantSP& key) const {
	ConstantSP newKey;
	if(keyCategory_==TEMPORAL && key->getType()!=keyType_)
		throw IncompatibleTypeException(keyType_, key->getType());
	newKey=key;

	ConstantSP result=createValues(newKey);
	if(newKey->isScalar()){
		auto it=dict_.find(newKey->getLong());
		if(it==dict_.end())
			(*swriter_)(nullVal_,result);
		else
			(*swriter_)(it->second,result);
	}
	else{
		int len=newKey->size();
		const int bufSize=Util::BUF_SIZE;
		long long buf[bufSize];
		const long long* pbuf;
		U8 value[bufSize];
		int start=0;
		int counts;
		unordered_map<long long,U8>::const_iterator it;
		auto end=dict_.end();
		while(start<len){
			counts=(std::min)(len-start,bufSize);
			pbuf=newKey->getLongConst(start,counts,buf);
			for(int i=0;i<counts;++i){
				it=dict_.find(pbuf[i]);
				value[i]=it==end?nullVal_:it->second;
			}
			(*vwriter_)(value,result,start,counts);
			start+=counts;
		}
		result->setNullFlag(result->hasNull());
	}
	return result;
}

void LongDictionary::contain(const ConstantSP& target, const ConstantSP& resultSP) const{
	if(target->isScalar()){
		resultSP->setBool(static_cast<char>(dict_.find(target->getLong())!=dict_.end()));
	}
	else{
		int len=target->size();
		const int bufSize=Util::BUF_SIZE;
		char ret[bufSize];
		char* pret;
		long long buf[bufSize];
		const long long* pbuf;
		int start=0;
		int counts;

		unordered_map<long long,U8>::const_iterator it;
		auto end=dict_.end();
		while(start<len){
			counts=(std::min)(len-start,bufSize);
			pbuf=target->getLongConst(start,counts,buf);
			pret=resultSP->getBoolBuffer(start,counts,ret);
			for(int i=0;i<counts;++i)
				pret[i]=static_cast<char>(dict_.find(pbuf[i])!=end);
			resultSP->setBool(start,counts,pret);
			start+=counts;
		}
	}
}

ConstantSP LongDictionary::keys() const {
	auto it=dict_.begin();
	int len=size();
	ConstantSP resultSP=Util::createVector(keyType_,len);
	int start=0;
	int counts;
	const int bufSize=Util::BUF_SIZE;
	long long buf[bufSize];
	long long* pbuf;

	while(start<len){
		counts=(std::min)(len-start,bufSize);
		pbuf=resultSP->getLongBuffer(start,counts,buf);
		for(int i=0;i<counts;++i){
			pbuf[i]=it->first;
			++it;
		}
		resultSP->setLong(start,counts,pbuf);
		start+=counts;
	}
	return resultSP;
}

ConstantSP LongDictionary::values() const {
	auto it=dict_.begin();
	int len=size();
	ConstantSP resultSP= Util::createVector(type_,len);

	int start=0;
	int counts;
	const int bufSize=Util::BUF_SIZE;
	U8 buf[bufSize];

	while(start<len){
		counts=(std::min)(len-start,bufSize);
		for(int i=0;i<counts;++i){
			buf[i]=it->second;
			++it;
		}
		(*vwriter_)(buf,resultSP,start,counts);
		start+=counts;
	}
	return resultSP;
}

std::string LongDictionary::getString() const {
	string content;
	int len=(std::min)(Util::DISPLAY_ROWS,(int)dict_.size());
	int counts=0;
	auto it=dict_.begin();
	ConstantSP key=Util::createConstant(keyType_);
	ConstantSP value=Util::createConstant(type_);
	while(counts<len){
		key->setLong(it->first);
		content.append(key->getString());
		content.append("->");
		(*swriter_)(it->second,value);
		content.append(value->getString());
		content.append(1,'\n');
		++counts;
		++it;
	}
	if(len<(int)dict_.size())
		content.append("...\n");
	return content;
}

long long LongDictionary::getAllocatedMemory() const {
	long long bytes=sizeof(LongDictionary)+(size()*16);
	if(getType()==DT_STRING){
		auto it=dict_.begin();
		auto end=dict_.end();
		while(it!=end){
			bytes += strlen(it->second.pointer);
			++it;
		}
	}
	return bytes;
}

FloatDictionary::FloatDictionary(const unordered_map<float,U8>& dict, DATA_TYPE type)
	:AbstractDictionary(DT_FLOAT,type),dict_(dict){
	if(type_!=DT_STRING)
		return;
	auto it=dict_.begin();
	auto end=dict_.end();
	size_t len;
	char* tmp;
	while(it!=end){
		len=strlen(it->second.pointer)+1;
		tmp=new char[len];
		memcpy(tmp,it->second.pointer,len);
		it->second.pointer=tmp;
		++it;
	}
}

FloatDictionary::~FloatDictionary(){
	if(type_ != DT_STRING && type_ != DT_BLOB)
		return;
	auto it=dict_.begin();
	auto end=dict_.end();
	while(it!=end){
		delete[] it->second.pointer;
		++it;
	}
}

bool FloatDictionary::remove(const ConstantSP& key){
	if(key->isScalar())
		dict_.erase(key->getFloat());
	else{
		int len=key->size();
		const int bufSize = Util::BUF_SIZE;
		float buf[bufSize];
		const float* pbuf;
		int counts;
		int start = 0;
		while(start < len){
			counts = (std::min)(len - start, bufSize);
			pbuf = key->getFloatConst(start, counts, buf);
			for(int i=0; i<counts; ++i)
				dict_.erase(pbuf[i]);
			start += counts;
		}
	}
	return true;
}

bool FloatDictionary::set(const ConstantSP& key, const ConstantSP& value){
	if(key->isScalar()){
		U8 tmp;
		(*sreader_)(value,tmp);
		dict_[key->getFloat()]=tmp;
		return true;
	}
	int len = key->size();
	if (len != value->size())
		return false;

	if (dict_.empty())
		dict_.reserve((int)(len * 1.33));
	const int bufSize = Util::BUF_SIZE;
	float buf[bufSize];
	const float *pbuf;
	U8 tmp[bufSize];
	int start = 0;
	int counts;
	std::size_t dictSize = dict_.size();
	while (start < len)
	{
		counts = (std::min)(len - start, bufSize);
		pbuf = key->getFloatConst(start, counts, buf);
		(*vreader_)(value, start, counts, tmp);
		if (type_ == DT_STRING) {
			for (int i = 0; i < counts; ++i) {
				U8 &cur = dict_[pbuf[i]];
				if (dict_.size() == dictSize)
					delete[] cur.pointer;
				else
					++dictSize;
				cur = tmp[i];
			}
		} else {
			for (int i = 0; i < counts; ++i)
				dict_[pbuf[i]] = tmp[i];
		}
		start += counts;
	}
	return true;
}

ConstantSP FloatDictionary::getMember(const ConstantSP& key) const {
	ConstantSP result=createValues(key);
	if(key->isScalar()){
		auto it=dict_.find(key->getFloat());
		if(it==dict_.end())
			(*swriter_)(nullVal_,result);
		else
			(*swriter_)(it->second,result);
	}
	else{
		int len=key->size();
		const int bufSize=Util::BUF_SIZE;
		float buf[bufSize];
		const float* pbuf;
		U8 value[bufSize];
		int start=0;
		int counts;
		unordered_map<float,U8>::const_iterator it;
		auto end=dict_.end();
		while(start<len){
			counts=(std::min)(len-start,bufSize);
			pbuf=key->getFloatConst(start,counts,buf);
			for(int i=0;i<counts;++i){
				it=dict_.find(pbuf[i]);
				value[i]=it==end?nullVal_:it->second;
			}
			(*vwriter_)(value,result,start,counts);
			start+=counts;
		}
		result->setNullFlag(result->hasNull());
	}
	return result;
}

void FloatDictionary::contain(const ConstantSP& target, const ConstantSP& resultSP) const{
	if(target->isScalar()){
		resultSP->setBool(static_cast<char>(dict_.find(target->getFloat())!=dict_.end()));
	}
	else{
		int len=target->size();
		const int bufSize = Util::BUF_SIZE;
		char ret[bufSize];
		char* pret;
		float buf[bufSize];
		const float* pbuf;
		int start=0;
		int counts;

		unordered_map<float,U8>::const_iterator it;
		auto end=dict_.end();
		while(start<len){
			counts=(std::min)(len-start,bufSize);
			pbuf=target->getFloatConst(start,counts,buf);
			pret=resultSP->getBoolBuffer(start,counts,ret);
			for(int i=0;i<counts;++i)
				pret[i]=static_cast<char>(dict_.find(pbuf[i])!=end);
			resultSP->setBool(start,counts,pret);
			start+=counts;
		}
	}
}

ConstantSP FloatDictionary::keys() const {
	auto it=dict_.begin();
	int len=size();
	ConstantSP resultSP=Util::createVector(keyType_,len);
	int start=0;
	int counts;
	const int bufSize = Util::BUF_SIZE;
	float buf[bufSize];
	float* pbuf;

	while(start<len){
		counts=(std::min)(len-start,bufSize);
		pbuf=resultSP->getFloatBuffer(start,counts,buf);
		for(int i=0;i<counts;++i){
			pbuf[i]=it->first;
			++it;
		}
		resultSP->setFloat(start,counts,pbuf);
		start+=counts;
	}
	return resultSP;
}

ConstantSP FloatDictionary::values() const {
	auto it=dict_.begin();
	int len=size();
	ConstantSP resultSP= Util::createVector(type_,len);

	int start=0;
	int counts;
	const int bufSize = Util::BUF_SIZE;
	U8 buf[bufSize];

	while(start<len){
		counts=(std::min)(len-start,bufSize);
		for(int i=0;i<counts;++i){
			buf[i]=it->second;
			++it;
		}
		(*vwriter_)(buf,resultSP,start,counts);
		start+=counts;
	}
	return resultSP;
}

std::string FloatDictionary::getString() const {
	string content;
	int len=(std::min)(Util::DISPLAY_ROWS,(int)dict_.size());
	int counts=0;
	auto it=dict_.begin();
	ConstantSP key=Util::createConstant(keyType_);
	ConstantSP value=Util::createConstant(type_);
	while(counts<len){
		key->setFloat(it->first);
		content.append(key->getString());
		content.append("->");
		(*swriter_)(it->second,value);
		content.append(value->getString());
		content.append(1,'\n');
		++counts;
		++it;
	}
	if(len<(int)dict_.size())
		content.append("...\n");
	return content;
}

long long FloatDictionary::getAllocatedMemory() const {
	long long bytes=sizeof(FloatDictionary)+(size()*12);
	if(getType()==DT_STRING){
		auto it=dict_.begin();
		auto end=dict_.end();
		while(it!=end){
			bytes += strlen(it->second.pointer);
			++it;
		}
	}
	return bytes;
}

DoubleDictionary::DoubleDictionary(const unordered_map<double,U8>& dict, DATA_TYPE type)
	:AbstractDictionary(DT_DOUBLE,type),dict_(dict){
	if(type_!=DT_STRING)
		return;
	auto it=dict_.begin();
	auto end=dict_.end();
	size_t len;
	char* tmp;
	while(it!=end){
		len=strlen(it->second.pointer)+1;
		tmp=new char[len];
		memcpy(tmp,it->second.pointer,len);
		it->second.pointer=tmp;
		++it;
	}
}

DoubleDictionary::~DoubleDictionary(){
	if(type_ != DT_STRING && type_ != DT_BLOB)
		return;
	auto it=dict_.begin();
	auto end=dict_.end();
	while(it!=end){
		delete[] it->second.pointer;
		++it;
	}
}

bool DoubleDictionary::remove(const ConstantSP& key){
	if(key->isScalar())
		dict_.erase(key->getDouble());
	else{
		int len=key->size();
		const int bufSize = Util::BUF_SIZE;
		double buf[bufSize];
		const double* pbuf;
		int counts;
		int start = 0;
		while(start < len){
			counts = (std::min)(len - start, bufSize);
			pbuf = key->getDoubleConst(start, counts, buf);
			for(int i=0; i<counts; ++i)
				dict_.erase(pbuf[i]);
			start += counts;
		}
	}
	return true;
}

bool DoubleDictionary::set(const ConstantSP& key, const ConstantSP& value){
	if(key->isScalar()){
		U8 tmp;
		(*sreader_)(value,tmp);
		dict_[key->getDouble()]=tmp;
		return true;
	}
		int len=key->size();
		if(len!=value->size())
			return false;

		if(dict_.empty())
			dict_.reserve((int)(len*1.33));
		const int bufSize = Util::BUF_SIZE;
		double buf[bufSize];
		const double* pbuf;
		U8 tmp[bufSize];
		int start=0;
		int counts;
		std::size_t dictSize=dict_.size();
		while(start<len){
			counts=(std::min)(len-start,bufSize);
			pbuf=key->getDoubleConst(start,counts,buf);
			(*vreader_)(value,start,counts,tmp);
			if(type_==DT_STRING){
				for(int i=0;i<counts;++i){
					U8& cur=dict_[pbuf[i]];
					if(dict_.size()==dictSize)
						delete[] cur.pointer;
					else
						++dictSize;
					cur=tmp[i];
				}
			}
			else{
				for(int i=0;i<counts;++i)
					dict_[pbuf[i]]=tmp[i];
			}
			start+=counts;
		}
		return true;
}

ConstantSP DoubleDictionary::getMember(const ConstantSP& key) const {
	ConstantSP result=createValues(key);
	if(key->isScalar()){
		auto it=dict_.find(key->getDouble());
		if(it==dict_.end())
			(*swriter_)(nullVal_,result);
		else
			(*swriter_)(it->second,result);
	}
	else{
		int len=key->size();
		const int bufSize = Util::BUF_SIZE;
		double buf[bufSize];
		const double* pbuf;
		U8 value[bufSize];
		int start=0;
		int counts;
		unordered_map<double,U8>::const_iterator it;
		auto end=dict_.end();
		while(start<len){
			counts=(std::min)(len-start,bufSize);
			pbuf=key->getDoubleConst(start,counts,buf);
			for(int i=0;i<counts;++i){
				it=dict_.find(pbuf[i]);
				value[i]=it==end?nullVal_:it->second;
			}
			(*vwriter_)(value,result,start,counts);
			start+=counts;
		}
		result->setNullFlag(result->hasNull());
	}
	return result;
}

void DoubleDictionary::contain(const ConstantSP& target, const ConstantSP& resultSP) const{
	if(target->isScalar()){
		resultSP->setBool(static_cast<char>(dict_.find(target->getDouble())!=dict_.end()));
	}
	else{
		int len=target->size();
		const int bufSize = Util::BUF_SIZE;
		char ret[bufSize];
		char* pret;
		double buf[bufSize];
		const double* pbuf;
		int start=0;
		int counts;

		unordered_map<double,U8>::const_iterator it;
		auto end=dict_.end();
		while(start<len){
			counts=(std::min)(len-start,bufSize);
			pbuf=target->getDoubleConst(start,counts,buf);
			pret=resultSP->getBoolBuffer(start,counts,ret);
			for(int i=0;i<counts;++i)
				pret[i]=static_cast<char>(dict_.find(pbuf[i])!=end);
			resultSP->setBool(start,counts,pret);
			start+=counts;
		}
	}
}

ConstantSP DoubleDictionary::keys() const {
	auto it=dict_.begin();
	int len=size();
	ConstantSP resultSP=Util::createVector(keyType_,len);
	int start=0;
	int counts;
	const int bufSize = Util::BUF_SIZE;
	double buf[bufSize];
	double* pbuf;

	while(start<len){
		counts=(std::min)(len-start,bufSize);
		pbuf=resultSP->getDoubleBuffer(start,counts,buf);
		for(int i=0;i<counts;++i){
			pbuf[i]=it->first;
			++it;
		}
		resultSP->setDouble(start,counts,pbuf);
		start+=counts;
	}
	return resultSP;
}

ConstantSP DoubleDictionary::values() const {
	auto it=dict_.begin();
	int len=size();
	ConstantSP resultSP= Util::createVector(type_,len);

	int start=0;
	int counts;
	const int bufSize = Util::BUF_SIZE;
	U8 buf[bufSize];

	while(start<len){
		counts=(std::min)(len-start,bufSize);
		for(int i=0;i<counts;++i){
			buf[i]=it->second;
			++it;
		}
		(*vwriter_)(buf,resultSP,start,counts);
		start+=counts;
	}
	return resultSP;
}

std::string DoubleDictionary::getString() const {
	string content;
	int len=(std::min)(Util::DISPLAY_ROWS,(int)dict_.size());
	int counts=0;
	auto it=dict_.begin();
	ConstantSP key=Util::createConstant(keyType_);
	ConstantSP value=Util::createConstant(type_);
	while(counts<len){
		key->setDouble(it->first);
		content.append(key->getString());
		content.append("->");
		(*swriter_)(it->second,value);
		content.append(value->getString());
		content.append(1,'\n');
		++counts;
		++it;
	}
	if(len<(int)dict_.size())
		content.append("...\n");
	return content;
}

long long DoubleDictionary::getAllocatedMemory() const {
	long long bytes=sizeof(DoubleDictionary)+(size()*16);
	if(getType()==DT_STRING){
		auto it=dict_.begin();
		auto end=dict_.end();
		while(it!=end){
			bytes += strlen(it->second.pointer);
			++it;
		}
	}
	return bytes;
}

StringDictionary::StringDictionary(const unordered_map<string,U8>& dict, DATA_TYPE keyType,DATA_TYPE type)
	:AbstractDictionary(keyType,type),dict_(dict){
	if(type_!=DT_STRING)
		return;
	auto it=dict_.begin();
	auto end=dict_.end();
	size_t len;
	char* tmp;
	while(it!=end){
		len=strlen(it->second.pointer)+1;
		tmp=new char[len];
		memcpy(tmp,it->second.pointer,len);
		it->second.pointer=tmp;
		++it;
	}
}

StringDictionary::~StringDictionary(){
	if(type_ != DT_STRING && type_ != DT_BLOB)
		return;
	auto it=dict_.begin();
	auto end=dict_.end();
	while(it!=end){
		delete[] it->second.pointer;
		++it;
	}
}

bool StringDictionary::remove(const ConstantSP& key){
	if(key->getCategory() != LITERAL && key->getType() != DT_BLOB)
		throw RuntimeException("Key data type incompatible. Expecting literal/BLOB data");
	if(key->isScalar())
		dict_.erase(key->getString());
	else{
		int len=key->size();
		const int bufSize = Util::BUF_SIZE;
		char* buf[bufSize];
		char** pbuf;
		int counts;
		int start = 0;
		while(start < len){
			counts = (std::min)(len - start, bufSize);
			pbuf = key->getStringConst(start, counts, buf);
			for(int i=0; i<counts; ++i)
				dict_.erase(pbuf[i]);
			start += counts;
		}
	}
	return true;
}

bool StringDictionary::set(const std::string & key, const ConstantSP& value){
	U8 tmp;
	(*sreader_)(value,tmp);
	dict_[key]=tmp;
	return true;
}

bool StringDictionary::set(const ConstantSP& key, const ConstantSP& value){
	if(key->getCategory() != LITERAL && key->getType() != DT_BLOB)
		throw RuntimeException("Key data type incompatible. Expecting literal/BLOB data");
	if(key->isScalar()){
		U8 tmp;
		(*sreader_)(value,tmp);
		dict_[key->getString()]=tmp;
		return true;
	}
		int len=key->size();
		if(len!=value->size())
			return false;

		if(dict_.empty())
			dict_.reserve((int)(len*1.33));
		const int bufSize = Util::BUF_SIZE;
		char* buf[bufSize];
		char** pbuf;
		U8 tmp[bufSize];
		int start=0;
		int counts;
		std::size_t dictSize=dict_.size();
		while(start<len){
			counts=(std::min)(len-start,bufSize);
			pbuf=key->getStringConst(start,counts,buf);
			(*vreader_)(value,start,counts,tmp);
			if(type_==DT_STRING){
				for(int i=0;i<counts;++i){
					U8& cur=dict_[pbuf[i]];
					if(dict_.size()==dictSize)
						delete[] cur.pointer;
					else
						++dictSize;
					cur=tmp[i];
				}
			}
			else{
				for(int i=0;i<counts;++i)
					dict_[pbuf[i]]=tmp[i];
			}
			start+=counts;
		}
		return true;
}

ConstantSP StringDictionary::getMember(const std::string & key) const {
	ConstantSP result = Util::createConstant(type_);
	auto it=dict_.find(key);
	if(it==dict_.end())
		(*swriter_)(nullVal_,result);
	else
		(*swriter_)(it->second,result);
	return result;
}

ConstantSP StringDictionary::getMember(const ConstantSP& key) const {
	if(key->getCategory() != LITERAL && key->getType() != DT_BLOB)
		throw RuntimeException("Key data type incompatible. Expecting literal/BLOB data");
	ConstantSP result=createValues(key);
	if(key->isScalar()){
		auto it=dict_.find(key->getStringRef());
		if(it==dict_.end())
			(*swriter_)(nullVal_,result);
		else
			(*swriter_)(it->second,result);
	}
	else{
		int len=key->size();
		const int bufSize = Util::BUF_SIZE;
		char* buf[bufSize];
		char** pbuf;
		U8 value[bufSize];
		int start=0;
		int counts;
		unordered_map<string,U8>::const_iterator it;
		auto end=dict_.end();
		while(start<len){
			counts=(std::min)(len-start,bufSize);
			pbuf=key->getStringConst(start,counts,buf);
			for(int i=0;i<counts;++i){
				it=dict_.find(pbuf[i]);
				value[i]=it==end?nullVal_:it->second;
			}
			(*vwriter_)(value,result,start,counts);
			start+=counts;
		}
		result->setNullFlag(result->hasNull());
	}
	return result;
}

void StringDictionary::contain(const ConstantSP& target, const ConstantSP& resultSP) const{
	if(target->getCategory() != LITERAL && target->getType() != DT_BLOB)
		throw RuntimeException("target data type incompatible. Expecting literal/BLOB data");
	if(target->isScalar()){
		resultSP->setBool(static_cast<char>(dict_.find(target->getStringRef())!=dict_.end()));
	}
	else{
		int len=target->size();
		const int bufSize = Util::BUF_SIZE;
		char ret[bufSize];
		char* pret;
		char* buf[bufSize];
		char** pbuf;
		int start=0;
		int counts;

		unordered_map<string,U8>::const_iterator it;
		auto end=dict_.end();
		while(start<len){
			counts=(std::min)(len-start,bufSize);
			pbuf=target->getStringConst(start,counts,buf);
			pret=resultSP->getBoolBuffer(start,counts,ret);
			for(int i=0;i<counts;++i)
				pret[i]=static_cast<char>(dict_.find(pbuf[i])!=end);
			resultSP->setBool(start,counts,pret);
			start+=counts;
		}
	}
}

ConstantSP StringDictionary::keys() const {
	auto it=dict_.begin();
	int len=size();
	ConstantSP resultSP=Util::createVector(keyType_,len);
	int start=0;
	int counts;
	const int bufSize = Util::BUF_SIZE;
	char* buf[bufSize];

	while(start<len){
		counts=(std::min)(len-start,bufSize);
		for(int i=0;i<counts;++i){
			buf[i]=(char*)it->first.c_str();
			++it;
		}
		resultSP->setString(start,counts,buf);
		start+=counts;
	}
	return resultSP;
}

ConstantSP StringDictionary::values() const {
	auto it=dict_.begin();
	int len=size();
	ConstantSP resultSP= Util::createVector(type_,len);

	int start=0;
	int counts;
	const int bufSize = Util::BUF_SIZE;
	U8 buf[bufSize];

	while(start<len){
		counts=(std::min)(len-start,bufSize);
		for(int i=0;i<counts;++i){
			buf[i]=it->second;
			++it;
		}
		(*vwriter_)(buf,resultSP,start,counts);
		start+=counts;
	}
	return resultSP;
}

std::string StringDictionary::getString() const {
	string content;
	int len=(std::min)(Util::DISPLAY_ROWS,(int)dict_.size());
	int counts=0;
	auto it=dict_.begin();
	ConstantSP value=Util::createConstant(type_);
	while(counts<len){
		content.append(it->first);
		content.append("->");
		(*swriter_)(it->second,value);
		content.append(value->getString());
		content.append(1,'\n');
		++counts;
		++it;
	}
	if(len<(int)dict_.size())
		content.append("...\n");
	return content;
}

long long StringDictionary::getAllocatedMemory() const {
	long long bytes=sizeof(StringDictionary)+(size()*(8+sizeof(string)));

	auto it=dict_.begin();
	auto end=dict_.end();
	if(getType()==DT_STRING){
		while(it!=end){
			bytes += strlen(it->second.pointer)+it->first.size();
			++it;
		}
	}
	else{
		while(it!=end){
			bytes += it->first.size();
			++it;
		}
	}
	return bytes;
}

bool AnyDictionary::remove(const ConstantSP& key){
	if(key->getCategory() != LITERAL && key->getType() != DT_BLOB)
		throw RuntimeException("Key data type incompatible. Expecting literal/BLOB data");
	if(key->isScalar())
		dict_.erase(key->getString());
	else{
		int len=key->size();
		const int bufSize = Util::BUF_SIZE;
		char* buf[bufSize];
		char** pbuf;
		int counts;
		int start = 0;
		while(start < len){
			counts = (std::min)(len - start, bufSize);
			pbuf = key->getStringConst(start, counts, buf);
			for(int i=0; i<counts; ++i)
				dict_.erase(pbuf[i]);
			start += counts;
		}
	}
	return true;
}

bool AnyDictionary::set(const std::string & key, const ConstantSP& value){
	dict_[key]=value;
	value->setIndependent(false);
	value->setTemporary(false);
	return true;
}

bool AnyDictionary::set(const ConstantSP& key, const ConstantSP& value){
	if(key->getCategory() != LITERAL && key->getCategory() != BINARY)
		throw RuntimeException("Dictionary with 'ANY' data type must use string or integer as key");
	if(key->isScalar()){
		dict_[key->getString()]=value;
		value->setIndependent(false);
		value->setTemporary(false);
	}
	else{
		int len=key->size();
		if(len!=value->size())
			throw RuntimeException("Key size doesn't match value size.");

		if(dict_.empty())
			dict_.reserve((int)(len*1.33));
		const int bufSize = Util::BUF_SIZE;
		char* buf[bufSize];
		char** pbuf;
		int start=0;
		int counts;
		while(start<len){
			counts=(std::min)(len-start,bufSize);
			pbuf=key->getStringConst(start,counts,buf);
			for(int i=0;i<counts;++i){
				ConstantSP obj = value->get(start + i);
				obj->setTemporary(false);
				obj->setIndependent(false);
				dict_[pbuf[i]] = obj;
			}
			start+=counts;
		}
	}
	return true;
}

ConstantSP AnyDictionary::getMember(const std::string & key) const{
	auto it=dict_.find(key);
	if(it==dict_.end())
		return Constant::void_;
	return it->second;
}

ConstantSP AnyDictionary::getMember(const ConstantSP& key) const {
	if(key->getCategory()!=LITERAL)
		throw RuntimeException("Dictionary with 'ANY' data type must use string as key");
	if(key->isScalar()){
		auto it=dict_.find(key->getString());
		if(it==dict_.end())
			return Constant::void_;
		return it->second;
	}
	ConstantSP result = Util::createVector(DT_ANY, key->size());
	int len = key->size();
	const int bufSize = Util::BUF_SIZE;
	char *buf[bufSize];
	char **pbuf;
	int start = 0;
	int counts;
	unordered_map<string, ConstantSP>::const_iterator it;
	auto end = dict_.end();
	while (start < len) {
		counts = (std::min)(len - start, bufSize);
		pbuf = key->getStringConst(start, counts, buf);
		for (int i = 0; i < counts; ++i) {
			it = dict_.find(pbuf[i]);
			result->set(start + i, it == end ? Constant::void_ : it->second);
		}
		start += counts;
	}
	result->setNullFlag(result->hasNull());
	return result;
}

ConstantSP AnyDictionary::keys() const {
	auto it=dict_.begin();
	int len=size();
	ConstantSP resultSP=Util::createVector(keyType_,len);
	int start=0;
	int counts;
	const int bufSize = Util::BUF_SIZE;
	char* buf[bufSize];

	while(start<len){
		counts=(std::min)(len-start,bufSize);
		for(int i=0;i<counts;++i){
			buf[i]=(char*)it->first.c_str();
			++it;
		}
		resultSP->setString(start,counts,buf);
		start+=counts;
	}
	return resultSP;
}

ConstantSP AnyDictionary::values() const {
	auto it=dict_.begin();
	int len=size();
	ConstantSP result=Util::createVector(DT_ANY,len);

	int counts = 0;
	while(it != dict_.end()){
		result->set(counts, it->second);
		++it;
		++counts;
	}
	return result;
}

void AnyDictionary::contain(const ConstantSP& target, const ConstantSP& resultSP) const{
	if(target->getCategory() != LITERAL && target->getType() != DT_BLOB)
		throw RuntimeException("Key data type incompatible. Expecting literal/BLOB data");
	if(target->isScalar()){
		resultSP->setBool(static_cast<char>(dict_.find(target->getStringRef())!=dict_.end()));
	}
	else{
		int len=target->size();
		const int bufSize = Util::BUF_SIZE;
		char ret[bufSize];
		char* pret;
		char* buf[bufSize];
		char** pbuf;
		int start=0;
		int counts;

		unordered_map<string,ConstantSP>::const_iterator it;
		auto end=dict_.end();
		while(start<len){
			counts=(std::min)(len-start,bufSize);
			pbuf=target->getStringConst(start,counts,buf);
			pret=resultSP->getBoolBuffer(start,counts,ret);
			for(int i=0;i<counts;++i)
				pret[i]=static_cast<char>(dict_.find(pbuf[i])!=end);
			resultSP->setBool(start,counts,pret);
			start+=counts;
		}
	}
}

std::string AnyDictionary::getString() const{
	string content;
	int len=(std::min)(Util::DISPLAY_ROWS,(int)dict_.size());
	int counts=0;
	auto it=dict_.begin();
	while(counts<len){
		content.append(it->first);
		content.append("->");
		DATA_FORM form = it->second->getForm();
		if(form == DF_MATRIX || form == DF_TABLE)
			content.append("\n");
		else if(form == DF_DICTIONARY)
			content.append("{\n");
		content.append(it->second->getString());
		if(form == DF_DICTIONARY)
			content.append("}");
		content.append(1,'\n');
		++counts;
		++it;
	}
	if(len<(int)dict_.size())
		content.append("...\n");
	return content;
}

long long AnyDictionary::getAllocatedMemory() const {
	long long bytes=sizeof(AnyDictionary)+(size()*(sizeof(ConstantSP)+sizeof(string)));

	auto it=dict_.begin();
	auto end=dict_.end();
	while(it!=end){
		bytes=it->first.size();
		if(it->second.count()==1)
			bytes+=it->second->getAllocatedMemory();
		++it;
	}
	return bytes;
}

bool AnyDictionary::containNotMarshallableObject() const{
	auto it=dict_.begin();
	auto end=dict_.end();
	while(it!=end){
		if((*it).second->containNotMarshallableObject())
			return true;
		++it;
	}
	return false;
}

bool IntAnyDictionary::remove(const ConstantSP& key){
	if(key->isScalar())
		dict_.erase(key->getInt());
	else{
		int len=key->size();
		const int bufSize = Util::BUF_SIZE;
		int buf[bufSize];
		const int* pbuf;
		int counts;
		int start = 0;
		while(start < len){
			counts = (std::min)(len - start, bufSize);
			pbuf = key->getIntConst(start, counts, buf);
			for(int i=0; i<counts; ++i)
				dict_.erase(pbuf[i]);
			start += counts;
		}
	}
	return true;
}

bool IntAnyDictionary::set(int key, const ConstantSP& value){
	dict_[key]=value;
	value->setIndependent(false);
	value->setTemporary(false);
	return true;
}

bool IntAnyDictionary::set(const ConstantSP& key, const ConstantSP& value){
	if(key->isScalar()){
		dict_[key->getInt()]=value;
		value->setIndependent(false);
		value->setTemporary(false);
	}
	else{
		int len=key->size();
		if(len!=value->size())
			return false;

		if(dict_.empty())
			dict_.reserve((int)(len*1.33));
		const int bufSize = Util::BUF_SIZE;
		int buf[bufSize];
		const int* pbuf;
		int start=0;
		int counts;
		while(start<len){
			counts=(std::min)(len-start,bufSize);
			pbuf=key->getIntConst(start,counts,buf);
			for(int i=0;i<counts;++i){
				ConstantSP obj = value->get(start+i);
				obj->setIndependent(false);
				obj->setTemporary(false);
				dict_[pbuf[i]] = obj;
			}
			start+=counts;
		}
	}
	return true;
}

ConstantSP IntAnyDictionary::getMember(const ConstantSP& key) const {
	if(key->isScalar()){
		auto it=dict_.find(key->getInt());
		if(it==dict_.end())
			return Constant::void_;
		return it->second;
	}
	ConstantSP result = Util::createVector(DT_ANY, key->size());
	int len = key->size();
	const int bufSize = Util::BUF_SIZE;
	int buf[bufSize];
	const int *pbuf;
	int start = 0;
	int counts;
	unordered_map<int, ConstantSP>::const_iterator it;
	auto end = dict_.end();
	while (start < len) {
		counts = (std::min)(len - start, bufSize);
		pbuf = key->getIntConst(start, counts, buf);
		for (int i = 0; i < counts; ++i) {
			it = dict_.find(pbuf[i]);
			result->set(start + i, it == end ? Constant::void_ : it->second);
		}
		start += counts;
	}
	result->setNullFlag(result->hasNull());
	return result;
}

ConstantSP IntAnyDictionary::keys() const {
	auto it=dict_.begin();
	int len=size();
	ConstantSP resultSP=Util::createVector(keyType_,len);
	int start=0;
	int counts;
	const int bufSize = Util::BUF_SIZE;
	int buf[bufSize];
	int* pbuf;

	while(start<len){
		counts=(std::min)(len-start,bufSize);
		pbuf=resultSP->getIntBuffer(start,counts,buf);
		for(int i=0;i<counts;++i){
			pbuf[i]=it->first;
			++it;
		}
		resultSP->setInt(start,counts,pbuf);
		start+=counts;
	}
	return resultSP;
}

ConstantSP IntAnyDictionary::values() const {
	auto it=dict_.begin();
	int len=size();
	ConstantSP result=Util::createVector(DT_ANY,len);

	int counts = 0;
	while(it != dict_.end()){
		result->set(counts, it->second);
		++it;
		++counts;
	}
	return result;
}

void IntAnyDictionary::contain(const ConstantSP& target, const ConstantSP& resultSP) const{
	if(target->isScalar()){
		resultSP->setBool(static_cast<char>(dict_.find(target->getInt())!=dict_.end()));
	}
	else{
		int len=target->size();
		const int bufSize = Util::BUF_SIZE;
		char ret[bufSize];
		char* pret;
		int buf[bufSize];
		const int* pbuf;
		int start=0;
		int counts;

		unordered_map<int,ConstantSP>::const_iterator it;
		auto end=dict_.end();
		while(start<len){
			counts=(std::min)(len-start,bufSize);
			pbuf=target->getIntConst(start,counts,buf);
			pret=resultSP->getBoolBuffer(start,counts,ret);
			for(int i=0;i<counts;++i)
				pret[i]=static_cast<char>(dict_.find(pbuf[i])!=end);
			resultSP->setBool(start,counts,pret);
			start+=counts;
		}
	}
}

std::string IntAnyDictionary::getString() const {
	string content;
	int len=(std::min)(Util::DISPLAY_ROWS,(int)dict_.size());
	int counts=0;
	ConstantSP key=Util::createConstant(keyType_);
	auto it=dict_.begin();
	while(counts<len){
		key->setInt(it->first);
		content.append(key->getString());
		content.append("->");
		DATA_FORM form = it->second->getForm();
		if(form == DF_MATRIX || form == DF_TABLE)
			content.append("\n");
		else if(form == DF_DICTIONARY)
			content.append("{\n");
		content.append(it->second->getString());
		if(form == DF_DICTIONARY)
			content.append("}");
		content.append(1,'\n');
		++counts;
		++it;
	}
	if(len<(int)dict_.size())
		content.append("...\n");
	return content;
}

long long IntAnyDictionary::getAllocatedMemory() const {
	long long bytes=sizeof(IntAnyDictionary)+(size()*(sizeof(ConstantSP)+4));

	auto it=dict_.begin();
	auto end=dict_.end();
	while(it!=end){
		if(it->second.count()==1)
			bytes+=it->second->getAllocatedMemory();
		++it;
	}
	return bytes;
}

bool IntAnyDictionary::containNotMarshallableObject() const{
	auto it=dict_.begin();
	auto end=dict_.end();
	while(it!=end){
		if((*it).second->containNotMarshallableObject())
			return true;
		++it;
	}
	return false;
}

bool FloatAnyDictionary::set(const ConstantSP& key, const ConstantSP& value){
    if(key->getRawType() != DT_FLOAT)
        throw RuntimeException("Key data type incompatible. Expecting Float");
    if(key->isScalar()){
        dict_[key->getFloat()] = value;
        value->setIndependent(false);
        value->setTemporary(false);
		return true;
    }

    int len = key->size();
    if (len != value->size() && value->size() != 1)
        return false;

    if (dict_.empty())
        dict_.reserve((std::size_t)(len * 1.33));
    int bufSize = std::min(len, Util::BUF_SIZE);
    std::vector<float> buf(bufSize);
    const float *pbuf;
    int start = 0, counts = 0;
    while (start < len) {
        counts = std::min(len - start, bufSize);
        pbuf = key->getFloatConst(start, counts, buf.data());
        for (int i = 0; i < counts; ++i) {
            ConstantSP obj = value->get(start + i);
            obj->setIndependent(false);
            obj->setTemporary(false);
            dict_[pbuf[i]] = obj;
        }
        start += counts;
    }
    return true;
}

bool FloatAnyDictionary::remove(const ConstantSP& key){
    if (key->isScalar()) {
        dict_.erase(key->getFloat());
        return true;
    }
    int len = key->size();
    int bufSize = std::min(len, Util::BUF_SIZE);
    std::vector<float> buf(bufSize);
    const float *pbuf = nullptr;
    int counts = 0, start = 0;
    while (start < len) {
        counts = std::min(len - start, bufSize);
        pbuf = key->getFloatConst(start, counts, buf.data());
        for (int i = 0; i < counts; ++i)
            dict_.erase(pbuf[i]);
        start += counts;
    }
    return true;
}

ConstantSP FloatAnyDictionary::getMember(const ConstantSP& key) const{
    if(key->getRawType() != DT_FLOAT)
        throw RuntimeException("Key data type incompatible. Expecting Float");
    if(key->isScalar()){
        auto it = dict_.find(key->getFloat());
        if(it == dict_.end())
            return Constant::void_;
        return it->second;
    }
	ConstantSP result = Util::createVector(DT_ANY, key->size());
	int len = key->size();
	int bufSize = std::min(len, Util::BUF_SIZE);
    std::vector<float> buf(bufSize);
	const float *pbuf = nullptr;
	int start = 0, counts = 0;
	unordered_map<float, ConstantSP>::const_iterator it;
	auto end = dict_.end();
	while (start < len) {
		counts = std::min(len - start, bufSize);
		pbuf = key->getFloatConst(start, counts, buf.data());
		for (int i = 0; i < counts; ++i) {
			it = dict_.find(pbuf[i]);
			result->set(start + i, it == end ? Constant::void_ : it->second);
		}
		start += counts;
	}
	result->setNullFlag(result->hasNull());
	return result;
}

ConstantSP FloatAnyDictionary::keys() const{
    auto it = dict_.begin();
    int len = size();
    ConstantSP resultSP = Util::createVector(keyType_, len);
    int start = 0, counts = 0;
    int bufSize = std::min(len, Util::BUF_SIZE);
    std::vector<float> buf(bufSize);
    float* pbuf = nullptr;

    while(start < len){
        counts = std::min(len - start, bufSize);
        pbuf = resultSP->getFloatBuffer(start, counts, buf.data());
        for(int i = 0; i < counts; ++i){
            pbuf[i] = it->first;
            ++it;
        }
        resultSP->setFloat(start, counts, pbuf);
        start += counts;
    }
    return resultSP;
}

ConstantSP FloatAnyDictionary::values() const{
    auto it = dict_.begin();
    int len = size();
    ConstantSP result = Util::createVector(DT_ANY, len);

    int counts = 0;
    while(it != dict_.end()){
        result->set(counts, it->second);
        ++it;
        ++counts;
    }
    return result;
}

void FloatAnyDictionary::contain(const ConstantSP &target, const ConstantSP &resultSP) const
{
    if (target->getRawType() != DT_FLOAT)
        throw RuntimeException("Key data type incompatible. Expecting Float");
    if (target->isScalar()) {
        resultSP->setBool(static_cast<char>(dict_.find(target->getFloat()) != dict_.end()));
        return;
    }
    int len = target->size();
    int bufSize = std::min(len, Util::BUF_SIZE);
    std::vector<char> ret(bufSize);
    char *pret = nullptr;
    std::vector<float> buf(bufSize);
    const float *pbuf = nullptr;
    int start = 0, counts = 0;

    auto end = dict_.end();
    while (start < len) {
        counts = std::min(len - start, bufSize);
        pbuf = target->getFloatConst(start, counts, buf.data());
        pret = resultSP->getBoolBuffer(start, counts, ret.data());
        for (int i = 0; i < counts; ++i)
            pret[i] = static_cast<char>(dict_.find(pbuf[i]) != end);
        resultSP->setBool(start, counts, pret);
        start += counts;
    }
}

std::string FloatAnyDictionary::getString() const{
    string content;
    int len=std::min(Util::DISPLAY_ROWS, (int)dict_.size());
    int counts = 0;
    ConstantSP key=Util::createConstant(keyType_);
    auto it = dict_.begin();
    while(counts < len){
        key->setFloat(it->first);
        content.append(key->getString());
        content.append("->");
        DATA_FORM form = it->second->getForm();
        if(form == DF_MATRIX || form == DF_TABLE)
            content.append("\n");
        else if(form == DF_DICTIONARY)
            content.append("{\n");
        content.append(it->second->getString());
        if(form == DF_DICTIONARY)
            content.append("}");
        content.append(1,'\n');
        ++counts;
        ++it;
    }
    if(len<(int)dict_.size())
        content.append("...\n");
    return content;
}

long long FloatAnyDictionary::getAllocatedMemory() const{
    long long bytes= sizeof(FloatAnyDictionary) + (size() * (sizeof(ConstantSP) + 8));
    auto it = dict_.begin();
    auto end = dict_.end();
    while(it != end){
        if(it->second.count() == 1)
            bytes += it->second->getAllocatedMemory();
        ++it;
    }
    return bytes;
}

bool FloatAnyDictionary::containNotMarshallableObject() const{
    auto it = dict_.begin();
    auto end = dict_.end();
    while(it != end){
        if((*it).second->containNotMarshallableObject())
            return true;
        ++it;
    }
    return false;
}

bool DoubleAnyDictionary::set(const ConstantSP &key, const ConstantSP &value)
{
    if (key->getRawType() != DT_DOUBLE)
        throw RuntimeException("Key data type incompatible. Expecting Double");
    if (key->isScalar()) {
        dict_[key->getDouble()] = value;
        value->setIndependent(false);
        value->setTemporary(false);
        return true;
    }
    int len = key->size();
    if (len != value->size() && value->size() != 1)
        return false;

    if (dict_.empty())
        dict_.reserve((std::size_t)(len * 1.33));
    int bufSize = std::min(len, Util::BUF_SIZE);
    std::vector<double> buf(bufSize);
    const double *pbuf = nullptr;
    int start = 0, counts = 0;
    while (start < len) {
        counts = std::min(len - start, bufSize);
        pbuf = key->getDoubleConst(start, counts, buf.data());
        for (int i = 0; i < counts; ++i) {
            ConstantSP obj = value->get(start + i);
            obj->setIndependent(false);
            obj->setTemporary(false);
            dict_[pbuf[i]] = obj;
        }
        start += counts;
    }
    return true;
}

bool DoubleAnyDictionary::remove(const ConstantSP &key)
{
    if (key->isScalar()) {
        dict_.erase(key->getDouble());
        return true;
    }
    int len = key->size();
    int bufSize = std::min(len, Util::BUF_SIZE);
    std::vector<double> buf(bufSize);
    const double *pbuf = nullptr;
    int counts = 0, start = 0;
    while (start < len) {
        counts = std::min(len - start, bufSize);
        pbuf = key->getDoubleConst(start, counts, buf.data());
        for (int i = 0; i < counts; ++i)
            dict_.erase(pbuf[i]);
        start += counts;
    }
    return true;
}

ConstantSP DoubleAnyDictionary::getMember(const ConstantSP& key) const{
    if(key->getRawType() != DT_DOUBLE)
        throw RuntimeException("Key data type incompatible. Expecting Double");
    if(key->isScalar()){
        auto it = dict_.find(key->getDouble());
        if(it == dict_.end())
            return Constant::void_;
        return it->second;
    }
	ConstantSP result = Util::createVector(DT_ANY, key->size());
	int len = key->size();
	int bufSize = std::min(len, Util::BUF_SIZE);
    std::vector<double> buf(bufSize);
	const double *pbuf = nullptr;
	int start = 0, counts = 0;
	unordered_map<double, ConstantSP>::const_iterator it;
	auto end = dict_.end();
	while (start < len) {
		counts = std::min(len - start, bufSize);
		pbuf = key->getDoubleConst(start, counts, buf.data());
		for (int i = 0; i < counts; ++i) {
			it = dict_.find(pbuf[i]);
			result->set(start + i, it == end ? Constant::void_ : it->second);
		}
		start += counts;
	}
	result->setNullFlag(result->hasNull());
	return result;
}

ConstantSP DoubleAnyDictionary::keys() const{
    auto it = dict_.begin();
    int len = size();
    ConstantSP resultSP = Util::createVector(keyType_, len);
    int start = 0, counts = 0;
    int bufSize = std::min(len, Util::BUF_SIZE);
    std::vector<double> buf(bufSize);
    double* pbuf = nullptr;

    while(start < len){
        counts = std::min(len - start, bufSize);
        pbuf = resultSP->getDoubleBuffer(start, counts, buf.data());
        for(int i = 0; i < counts; ++i){
            pbuf[i] = it->first;
            ++it;
        }
        resultSP->setDouble(start, counts, pbuf);
        start += counts;
    }
    return resultSP;
}

ConstantSP DoubleAnyDictionary::values() const{
    auto it = dict_.begin();
    int len = size();
    ConstantSP result = Util::createVector(DT_ANY, len);

    int counts = 0;
    while(it != dict_.end()){
        result->set(counts, it->second);
        ++it;
        ++counts;
    }
    return result;
}

void DoubleAnyDictionary::contain(const ConstantSP &target, const ConstantSP &resultSP) const
{
    if (target->getRawType() != DT_DOUBLE)
        throw RuntimeException("Key data type incompatible. Expecting Double");
    if (target->isScalar()) {
        resultSP->setBool(static_cast<char>(dict_.find(target->getDouble()) != dict_.end()));
        return;
    }
    int len = target->size();
    int bufSize = std::min(len, Util::BUF_SIZE);
    std::vector<char> ret(bufSize);
    char *pret = nullptr;
    std::vector<double> buf(bufSize);
    const double *pbuf = nullptr;
    int start = 0, counts = 0;

    auto end = dict_.end();
    while (start < len) {
        counts = std::min(len - start, bufSize);
        pbuf = target->getDoubleConst(start, counts, buf.data());
        pret = resultSP->getBoolBuffer(start, counts, ret.data());
        for (int i = 0; i < counts; ++i)
            pret[i] = static_cast<char>(dict_.find(pbuf[i]) != end);
        resultSP->setBool(start, counts, pret);
        start += counts;
    }
}

std::string DoubleAnyDictionary::getString() const{
    string content;
    int len=std::min(Util::DISPLAY_ROWS, (int)dict_.size());
    int counts = 0;
    ConstantSP key=Util::createConstant(keyType_);
    auto it = dict_.begin();
    while(counts < len){
        key->setDouble(it->first);
        content.append(key->getString());
        content.append("->");
        DATA_FORM form = it->second->getForm();
        if(form == DF_MATRIX || form == DF_TABLE)
            content.append("\n");
        else if(form == DF_DICTIONARY)
            content.append("{\n");
        content.append(it->second->getString());
        if(form == DF_DICTIONARY)
            content.append("}");
        content.append(1,'\n');
        ++counts;
        ++it;
    }
    if(len<(int)dict_.size())
        content.append("...\n");
    return content;
}

long long DoubleAnyDictionary::getAllocatedMemory() const{
    long long bytes= sizeof(DoubleAnyDictionary) + (size() * (sizeof(ConstantSP) + 8));
    auto it = dict_.begin();
    auto end = dict_.end();
    while(it != end){
        if(it->second.count() == 1)
            bytes += it->second->getAllocatedMemory();
        ++it;
    }
    return bytes;
}

bool DoubleAnyDictionary::containNotMarshallableObject() const{
    auto it = dict_.begin();
    auto end = dict_.end();
    while(it != end){
        if((*it).second->containNotMarshallableObject())
            return true;
        ++it;
    }
    return false;
}

bool LongAnyDictionary::remove(const ConstantSP &key)
{
    if (key->isScalar()) {
        dict_.erase(key->getLong());
        return true;
    }
    int len = key->size();
    int bufSize = std::min(len, Util::BUF_SIZE);
    std::vector<long long> buf(bufSize);
    const long long *pbuf;
    int counts;
    int start = 0;
    while (start < len) {
        counts = std::min(len - start, bufSize);
        pbuf = key->getLongConst(start, counts, buf.data());
        for (int i = 0; i < counts; ++i)
            dict_.erase(pbuf[i]);
        start += counts;
    }
    return true;
}

bool LongAnyDictionary::set(const ConstantSP &key, const ConstantSP &value)
{
    if (key->getRawType() != DT_LONG)
        throw RuntimeException("Key data type incompatible. Expecting LONG or the like");
    if (key->isScalar()) {
        dict_[key->getLong()] = value;
        value->setIndependent(false);
        value->setTemporary(false);
        return true;
    }
    int len = key->size();
    if (len != value->size())
        return false;

    if (dict_.empty())
        dict_.reserve((int)(len * 1.33));
    int bufSize = std::min(len, Util::BUF_SIZE);
    std::vector<long long> buf(bufSize);
    const long long *pbuf;
    int start = 0;
    int counts;
    while (start < len) {
        counts = std::min(len - start, bufSize);
        pbuf = key->getLongConst(start, counts, buf.data());
        for (int i = 0; i < counts; ++i) {
            ConstantSP obj = value->get(start + i);
            obj->setIndependent(false);
            obj->setTemporary(false);
            dict_[pbuf[i]] = obj;
        }
        start += counts;
    }
    return true;
}

ConstantSP LongAnyDictionary::getMember(const ConstantSP& key) const {
	if(key->getRawType() != DT_LONG)
		throw RuntimeException("Key data type incompatible. Expecting LONG or the like");
	if(key->isScalar()){
		auto it=dict_.find(key->getLong());
		if(it==dict_.end())
			return Constant::void_;
		return it->second;
	}
	ConstantSP result = Util::createVector(DT_ANY, key->size());
	int len = key->size();
	int bufSize = std::min(len, Util::BUF_SIZE);
    std::vector<long long> buf(bufSize);
	const long long *pbuf;
	int start = 0;
	int counts;
	unordered_map<long long, ConstantSP>::const_iterator it;
	auto end = dict_.end();
	while (start < len) {
		counts = std::min(len - start, bufSize);
		// pbuf=key->getLongConst(start,counts,buf);
		pbuf = key->getLongConst(start, counts, buf.data());
		for (int i = 0; i < counts; ++i) {
			it = dict_.find(pbuf[i]);
			result->set(start + i, it == end ? Constant::void_ : it->second);
		}
		start += counts;
	}
	result->setNullFlag(result->hasNull());
	return result;
}

ConstantSP LongAnyDictionary::keys() const {
	auto it=dict_.begin();
	int len=size();
	ConstantSP resultSP=Util::createVector(keyType_,len);
	int start=0;
	int counts;
	int bufSize=std::min(len,Util::BUF_SIZE);
    std::vector<long long> buf(bufSize);
	long long* pbuf;

	while(start<len){
		counts=std::min(len-start,bufSize);
		pbuf=resultSP->getLongBuffer(start,counts,buf.data());
		for(int i=0;i<counts;++i){
			pbuf[i]=it->first;
			++it;
		}
		resultSP->setLong(start,counts,pbuf);
		start+=counts;
	}
	return resultSP;
}

ConstantSP LongAnyDictionary::values() const {
	auto it=dict_.begin();
	int len=size();
	ConstantSP result=Util::createVector(DT_ANY,len);

	int counts = 0;
	while(it != dict_.end()){
		result->set(counts, it->second);
		++it;
		++counts;
	}
	return result;
}

void LongAnyDictionary::contain(const ConstantSP &target, const ConstantSP &resultSP) const
{
    if (target->getRawType() != DT_LONG)
        throw RuntimeException("Key data type incompatible. Expecting LONG or the like");
    if (target->isScalar()) {
        resultSP->setBool(static_cast<char>(dict_.find(target->getLong()) != dict_.end()));
        return;
    }
    int len = target->size();
    int bufSize = std::min(len, Util::BUF_SIZE);
    std::vector<char> ret(bufSize);
    char *pret;
    std::vector<long long> buf(bufSize);
    const long long *pbuf;
    int start = 0;
    int counts;

    unordered_map<long long, ConstantSP>::const_iterator it;
    auto end = dict_.end();
    while (start < len) {
        counts = std::min(len - start, bufSize);
        pbuf = target->getLongConst(start, counts, buf.data());
        pret = resultSP->getBoolBuffer(start, counts, ret.data());
        for (int i = 0; i < counts; ++i)
            pret[i] = static_cast<char>(dict_.find(pbuf[i]) != end);
        resultSP->setBool(start, counts, pret);
        start += counts;
    }
}

std::string LongAnyDictionary::getString() const {
	string content;
	int len=std::min(Util::DISPLAY_ROWS,(int)dict_.size());
	int counts=0;
	ConstantSP key=Util::createConstant(keyType_);
	auto it=dict_.begin();
	while(counts<len){
		key->setLong(it->first);
		content.append(key->getString());
		content.append("->");
		DATA_FORM form = it->second->getForm();
		if(form == DF_MATRIX || form == DF_TABLE)
			content.append("\n");
		else if(form == DF_DICTIONARY)
			content.append("{\n");
		content.append(it->second->getString());
		if(form == DF_DICTIONARY)
			content.append("}");
		content.append(1,'\n');
		++counts;
		++it;
	}
	if(len<(int)dict_.size())
		content.append("...\n");
	return content;
}

long long LongAnyDictionary::getAllocatedMemory() const {
	long long bytes=sizeof(LongAnyDictionary)+(size()*(sizeof(ConstantSP)+8));

	auto it=dict_.begin();
	auto end=dict_.end();
	while(it!=end){
		if(it->second.count()==1)
			bytes+=it->second->getAllocatedMemory();
		++it;
	}
	return bytes;
}

bool LongAnyDictionary::containNotMarshallableObject() const{
	auto it=dict_.begin();
	auto end=dict_.end();
	while(it!=end){
		if((*it).second->containNotMarshallableObject())
			return true;
		++it;
	}
	return false;
}


Int128Dictionary::Int128Dictionary(const unordered_map<Guid,U8>& dict, DATA_TYPE keyType,DATA_TYPE type)
	:AbstractDictionary(keyType,type),dict_(dict){
	if(type_!=DT_STRING)
		return;
	auto it=dict_.begin();
	auto end=dict_.end();
	size_t len;
	char* tmp;
	while(it!=end){
		len=strlen(it->second.pointer)+1;
		tmp=new char[len];
		memcpy(tmp,it->second.pointer,len);
		it->second.pointer=tmp;
		++it;
	}
}

Int128Dictionary::~Int128Dictionary(){
	if(type_ != DT_STRING && type_ != DT_BLOB)
		return;
	auto it=dict_.begin();
	auto end=dict_.end();
	while(it!=end){
		delete[] it->second.pointer;
		++it;
	}
}

bool Int128Dictionary::remove(const ConstantSP &key)
{
    if (key->isScalar()) {
        dict_.erase(key->getInt128());
        return true;
    }
    int len = key->size();
    int bufSize = std::min(len, Util::BUF_SIZE);
    std::vector<unsigned char> buf(bufSize * 16);
    const Guid *pbuf;
    int counts;
    int start = 0;
    while (start < len) {
        counts = std::min(len - start, bufSize);
        pbuf = (const Guid *)key->getBinaryConst(start, counts, 16, buf.data());
        for (int i = 0; i < counts; ++i)
            dict_.erase(pbuf[i]);
        start += counts;
    }
    return true;
}

bool Int128Dictionary::set(const ConstantSP& key, const ConstantSP& value){
	if(key->isScalar()){
		U8 tmp;
		(*sreader_)(value,tmp);
		dict_[key->getInt128()]=tmp;
		return true;
	}
	int len = key->size();
	if (len != value->size())
		return false;

	if (dict_.empty())
		dict_.reserve((int)(len * 1.33));
	int bufSize = std::min(len, Util::BUF_SIZE);
    std::vector<unsigned char> buf(bufSize * 16);
	const Guid *pbuf;
    std::vector<U8> tmp(bufSize);
	int start = 0;
	int counts;
	std::size_t dictSize = dict_.size();
	while (start < len) {
		counts = std::min(len - start, bufSize);
		pbuf = (const Guid *)key->getBinaryConst(start, counts, 16, buf.data());
		(*vreader_)(value, start, counts, tmp.data());
		if (type_ == DT_STRING) {
			for (int i = 0; i < counts; ++i) {
				U8 &cur = dict_[pbuf[i]];
				if (dict_.size() == dictSize)
					delete[] cur.pointer;
				else
					++dictSize;
				cur = tmp[i];
			}
		} else {
			for (int i = 0; i < counts; ++i)
				dict_[pbuf[i]] = tmp[i];
		}
		start += counts;
	}
	return true;
}

ConstantSP Int128Dictionary::getMember(const ConstantSP &key) const
{
    ConstantSP result = createValues(key);
    if (key->isScalar()) {
        auto it = dict_.find(key->getInt128());
        if (it == dict_.end())
            (*swriter_)(nullVal_, result);
        else
            (*swriter_)(it->second, result);
        return result;
    }
    int len = key->size();
    int bufSize = std::min(len, Util::BUF_SIZE);
    std::vector<unsigned char> buf(bufSize * 16);
    const Guid *pbuf;
    std::vector<U8> value(bufSize);
    int start = 0;
    int counts;
    unordered_map<Guid, U8>::const_iterator it;
    auto end = dict_.end();
    while (start < len) {
        counts = std::min(len - start, bufSize);
        pbuf = (const Guid *)key->getBinaryConst(start, counts, 16, buf.data());
        for (int i = 0; i < counts; ++i) {
            it = dict_.find(pbuf[i]);
            value[i] = it == end ? nullVal_ : it->second;
        }
        (*vwriter_)(value.data(), result, start, counts);
        start += counts;
    }
    result->setNullFlag(result->hasNull());
    return result;
}

void Int128Dictionary::contain(const ConstantSP& target, const ConstantSP& resultSP) const{
	if(target->isScalar()){
		resultSP->setBool(static_cast<char>(dict_.find(target->getInt128())!=dict_.end()));
	}
	else{
		int len=target->size();
		int bufSize=std::min(len,Util::BUF_SIZE);
		std::unique_ptr<char[]> ret(new char[bufSize]); //char ret[bufSize];
		char* pret;
		std::unique_ptr<unsigned char[]> buf(new unsigned char[bufSize * 16]); //unsigned char buf[bufSize*16];
		const Guid* pbuf;
		int start=0;
		int counts;

		unordered_map<Guid,U8>::const_iterator it;
		auto end=dict_.end();
		while(start<len){
			counts=std::min(len-start,bufSize);
			pbuf=(const Guid*)target->getBinaryConst(start,counts,16,buf.get());
			pret=resultSP->getBoolBuffer(start,counts,ret.get());
			for(int i=0;i<counts;++i)
				pret[i]=static_cast<char>(dict_.find(pbuf[i])!=end);
			resultSP->setBool(start,counts,pret);
			start+=counts;
		}
	}
}

ConstantSP Int128Dictionary::keys() const {
	auto it=dict_.begin();
	int len=size();
	ConstantSP resultSP=Util::createVector(keyType_,len);
	int start=0;
	int counts;
	int bufSize=std::min(len,Util::BUF_SIZE);
    std::vector<unsigned char> buf(bufSize * 16);
	Guid* pbuf;

	while(start<len){
		counts=std::min(len-start,bufSize);
		pbuf=(Guid*)resultSP->getBinaryBuffer(start,counts,16,buf.data());
		for(int i=0;i<counts;++i){
			pbuf[i]=it->first;
			++it;
		}
		resultSP->setBinary(start,counts,16, (const unsigned char*)pbuf);
		start+=counts;
	}
	return resultSP;
}

ConstantSP Int128Dictionary::values() const {
	auto it=dict_.begin();
	int len=size();
	ConstantSP resultSP= Util::createVector(type_,len);

	int start=0;
	int counts;
	int bufSize=std::min(len,Util::BUF_SIZE);
    std::vector<U8> buf(bufSize);

	while(start<len){
		counts=std::min(len-start,bufSize);
		for(int i=0;i<counts;++i){
			buf[i]=it->second;
			++it;
		}
		(*vwriter_)(buf.data(),resultSP,start,counts);
		start+=counts;
	}
	return resultSP;
}

std::string Int128Dictionary::getString() const {
	string content;
	int len=std::min(Util::DISPLAY_ROWS,(int)dict_.size());
	int counts=0;
	auto it=dict_.begin();
	ConstantSP key=Util::createConstant(keyType_);
	ConstantSP value=Util::createConstant(type_);
	while(counts<len){
		content.append(it->first.getString());
		content.append("->");
		(*swriter_)(it->second,value);
		content.append(value->getString());
		content.append(1,'\n');
		++counts;
		++it;
	}
	if(len<(int)dict_.size())
		content.append("...\n");
	return content;
}

long long Int128Dictionary::getAllocatedMemory() const {
	long long bytes=sizeof(Int128Dictionary)+(size()*24);
	if(getType()==DT_STRING){
		auto it=dict_.begin();
		auto end=dict_.end();
		while(it!=end){
			bytes += strlen(it->second.pointer);
			++it;
		}
	}
	return bytes;
}


bool Int128AnyDictionary::remove(const ConstantSP& key){
	if(key->getRawType() != DT_INT128)
		throw RuntimeException("Key data type incompatible. Expecting INT128 or the like.");
	if(key->isScalar())
		dict_.erase(key->getInt128());
	else{
		int len=key->size();
		int bufSize=std::min(len,Util::BUF_SIZE);
		std::unique_ptr<unsigned char[]> buf(new unsigned char[bufSize * 16]); //unsigned char buf[bufSize*16];
		const Guid* pbuf;
		int counts;
		int start = 0;
		while(start < len){
			counts = std::min(len - start, bufSize);
			pbuf = (const Guid*)key->getBinaryConst(start, counts, 16, buf.get());
			for(int i=0; i<counts; ++i)
				dict_.erase(pbuf[i]);
			start += counts;
		}
	}
	return true;
}

bool Int128AnyDictionary::set(const ConstantSP& key, const ConstantSP& value){
	if(key->getRawType() != DT_INT128)
		throw RuntimeException("Key data type incompatible. Expecting INT128 or the like.");
	if(key->isScalar()){
		dict_[key->getInt128()]=value;
		value->setIndependent(false);
		value->setTemporary(false);
	} else{
		int len=key->size();
		if(len!=value->size())
			return false;

		if(dict_.empty())
			dict_.reserve((int)(len*1.33));
		int bufSize=std::min(len,Util::BUF_SIZE);
		std::unique_ptr<unsigned char[]> buf(new unsigned char[bufSize * 16]); //unsigned char buf[bufSize*16];
		const Guid* pbuf;
		int start=0;
		int counts;
		while(start<len){
			counts=std::min(len-start,bufSize);
			pbuf=(const Guid*)key->getBinaryConst(start,counts,16,buf.get());
			for(int i=0;i<counts;++i){
				ConstantSP obj = value->get(start+i);
				obj->setIndependent(false);
				obj->setTemporary(false);
				dict_[pbuf[i]] = obj;
			}
			start+=counts;
		}
	}
	return true;
}

ConstantSP Int128AnyDictionary::getMember(const ConstantSP& key) const {
	if(key->getRawType() != DT_INT128)
		throw RuntimeException("Key data type incompatible. Expecting INT128 or the like.");
	if(key->isScalar()){
		auto it=dict_.find(key->getInt128());
		if(it==dict_.end())
			return Constant::void_;
		return it->second;
	}
	ConstantSP result = Util::createVector(DT_ANY, key->size());
	int len = key->size();
	int bufSize = std::min(len, Util::BUF_SIZE);
	std::unique_ptr<unsigned char[]> buf(new unsigned char[bufSize * 16]); // unsigned char buf[bufSize*16];
	const Guid *pbuf;
	int start = 0;
	int counts;
	unordered_map<Guid, ConstantSP>::const_iterator it;
	auto end = dict_.end();
	while (start < len) {
		counts = std::min(len - start, bufSize);
		pbuf = (const Guid *)key->getBinaryConst(start, counts, 16, buf.get());
		for (int i = 0; i < counts; ++i) {
			it = dict_.find(pbuf[i]);
			result->set(start + i, it == end ? Constant::void_ : it->second);
		}
		start += counts;
	}
	result->setNullFlag(result->hasNull());
	return result;
}

ConstantSP Int128AnyDictionary::keys() const {
	auto it=dict_.begin();
	int len=size();
	ConstantSP resultSP=Util::createVector(keyType_,len);
	int start=0;
	int counts;
	int bufSize=std::min(len,Util::BUF_SIZE);
    std::vector<unsigned char> buf(bufSize * 16);
	Guid* pbuf;

	while(start<len){
		counts=std::min(len-start,bufSize);
		pbuf = (Guid*)resultSP->getBinaryBuffer(start,counts,16,buf.data());
		for(int i=0;i<counts;++i){
			pbuf[i]=it->first;
			++it;
		}
		resultSP->setBinary(start, counts, 16, (unsigned char*)pbuf);
		start+=counts;
	}
	return resultSP;
}

ConstantSP Int128AnyDictionary::values() const {
	auto it=dict_.begin();
	int len=size();
	ConstantSP result=Util::createVector(DT_ANY,len);

	int counts = 0;
	while(it != dict_.end()){
		result->set(counts, it->second);
		++it;
		++counts;
	}
	return result;
}

void Int128AnyDictionary::contain(const ConstantSP& target, const ConstantSP& resultSP) const{
	if(target->getRawType() != DT_INT128)
		throw RuntimeException("Key data type incompatible. Expecting INT128 or the like.");
	if(target->isScalar()){
		resultSP->setBool(static_cast<char>(dict_.find(target->getInt128())!=dict_.end()));
	}
	else{
		int len=target->size();
		int bufSize=std::min(len,Util::BUF_SIZE);
		std::unique_ptr<char[]> ret(new char[bufSize]); //char ret[bufSize];
		char* pret;
		std::unique_ptr<unsigned char[]> buf(new unsigned char[bufSize * 16]); //unsigned char buf[bufSize*16];
		const Guid* pbuf;
		int start=0;
		int counts;

		unordered_map<Guid,ConstantSP>::const_iterator it;
		auto end=dict_.end();
		while(start<len){
			counts=std::min(len-start,bufSize);
			pbuf= (const Guid*)target->getBinaryConst(start, counts, 16, buf.get());
			pret=resultSP->getBoolBuffer(start,counts,ret.get());
			for(int i=0;i<counts;++i)
				pret[i]=static_cast<char>(dict_.find(pbuf[i])!=end);
			resultSP->setBool(start,counts,pret);
			start+=counts;
		}
	}
}

std::string Int128AnyDictionary::getString() const {
	string content;
	int len=std::min(Util::DISPLAY_ROWS,(int)dict_.size());
	int counts=0;
	ConstantSP key=Util::createConstant(keyType_);
	auto it=dict_.begin();
	while(counts<len){
		key->setBinary(it->first.bytes(), 16);
		content.append(key->getString());
		content.append("->");
		DATA_FORM form = it->second->getForm();
		if(form == DF_MATRIX || form == DF_TABLE)
			content.append("\n");
		else if(form == DF_DICTIONARY)
			content.append("{\n");
		content.append(it->second->getString());
		if(form == DF_DICTIONARY)
			content.append("}");
		content.append(1,'\n');
		++counts;
		++it;
	}
	if(len<(int)dict_.size())
		content.append("...\n");
	return content;
}

long long Int128AnyDictionary::getAllocatedMemory() const {
	long long bytes=sizeof(IntAnyDictionary)+(size()*(sizeof(ConstantSP)+16));

	auto it=dict_.begin();
	auto end=dict_.end();
	while(it!=end){
		if(it->second.count()==1)
			bytes+=it->second->getAllocatedMemory();
		++it;
	}
	return bytes;
}

bool Int128AnyDictionary::containNotMarshallableObject() const{
	auto it=dict_.begin();
	auto end=dict_.end();
	while(it!=end){
		if((*it).second->containNotMarshallableObject())
			return true;
		++it;
	}
	return false;
}

} // namespace dolphindb
