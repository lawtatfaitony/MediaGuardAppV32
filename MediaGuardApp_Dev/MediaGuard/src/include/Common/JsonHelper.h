/**************************************************************
* @brief:       functions of JSON 
* @copyright:
*
***************************************************************/
#pragma once
#include "rapidjson/document.h"
#include "rapidjson/writer.h"

#include"rapidjson/reader.h"
#include"rapidjson/schema.h"
#include"rapidjson/stream.h"
#include"rapidjson/stringbuffer.h" 

namespace JsonHelper
{
	// generate list value
	template<class TList>
	static void GenerateList(int nPage, int nPageSize, int64_t nCnt, const TList& listData, std::string& strRst)
	{
		rapidjson::Document doc(rapidjson::kObjectType);
		rapidjson::Document::AllocatorType& typeAllocate = doc.GetAllocator();
		doc.AddMember("code", 0, typeAllocate);
		// info
		{
			rapidjson::Value valueInfo(rapidjson::kObjectType);
			rapidjson::Value valueArr(rapidjson::kArrayType);
			for (const auto& item : listData)
			{
				rapidjson::Value valueData(rapidjson::kObjectType);
				to_json(item, valueData, typeAllocate);
				valueArr.PushBack(valueData, typeAllocate);
			}
			valueInfo.AddMember("page_no", nPage, typeAllocate);
			valueInfo.AddMember("page_size", nPageSize, typeAllocate);
			valueInfo.AddMember("total_count", nCnt, typeAllocate);
			valueInfo.AddMember("items", valueArr, typeAllocate);

			doc.AddMember("info", valueInfo, typeAllocate);
		}
		doc.AddMember("msg", rapidjson::Value("", typeAllocate), typeAllocate);

		rapidjson::StringBuffer buffer;
		rapidjson::Writer<rapidjson::StringBuffer>  writer(buffer);
		doc.Accept(writer);
		strRst = buffer.GetString();
	}

	// generate list value
	template<class TList>
	static void GenerateListResult(int64_t nCnt, const TList& listData, std::string& strRst)
	{
		rapidjson::Document doc(rapidjson::kObjectType);
		rapidjson::Document::AllocatorType& typeAllocate = doc.GetAllocator();
		doc.AddMember("code", 0, typeAllocate);
		// info
		{
			rapidjson::Value valueInfo(rapidjson::kObjectType);
			rapidjson::Value valueArr(rapidjson::kArrayType);
			for (const auto& item : listData)
			{
				rapidjson::Value valueData(rapidjson::kObjectType);
				to_json(item, valueData, typeAllocate);
				valueArr.PushBack(valueData, typeAllocate);
			}
			valueInfo.AddMember("total_count", nCnt, typeAllocate);
			valueInfo.AddMember("items", valueArr, typeAllocate);

			doc.AddMember("info", valueInfo, typeAllocate);
		}
		doc.AddMember("msg", rapidjson::Value("", typeAllocate), typeAllocate);

		rapidjson::StringBuffer buffer;
		rapidjson::Writer<rapidjson::StringBuffer>  writer(buffer);
		doc.Accept(writer);
		strRst = buffer.GetString();
	}

	// generate int value
	template<class T>
	static void GenerateValue(const std::string& strKey, T nValue, std::string& strRst)
	{
		rapidjson::Document doc(rapidjson::kObjectType);
		rapidjson::Document::AllocatorType& typeAllocate = doc.GetAllocator();
		doc.AddMember("code", 0, typeAllocate);
		// info
		{
			rapidjson::Value valueInfo(rapidjson::kObjectType);
			valueInfo.AddMember(rapidjson::Value(strKey.c_str(), typeAllocate), nValue, typeAllocate);

			doc.AddMember("info", valueInfo, typeAllocate);
		}
		doc.AddMember("msg", rapidjson::Value("", typeAllocate), typeAllocate);

		rapidjson::StringBuffer buffer;
		rapidjson::Writer<rapidjson::StringBuffer>  writer(buffer);
		doc.Accept(writer);
		strRst = buffer.GetString();
	}

	// generate string value
	static void GenerateStringValue(const std::string& strKey, const std::string& strValue, std::string& strRst)
	{
		rapidjson::Document doc(rapidjson::kObjectType);
		rapidjson::Document::AllocatorType& typeAllocate = doc.GetAllocator();
		doc.AddMember("code", 0, typeAllocate);
		// info
		{
			rapidjson::Value valueInfo(rapidjson::kObjectType);
			valueInfo.AddMember(rapidjson::Value(strKey.c_str(), typeAllocate), rapidjson::Value(strValue.c_str(), typeAllocate), typeAllocate);

			doc.AddMember("info", valueInfo, typeAllocate);
		}
		doc.AddMember("msg", rapidjson::Value("", typeAllocate), typeAllocate);

		rapidjson::StringBuffer buffer;
		rapidjson::Writer<rapidjson::StringBuffer>  writer(buffer);
		doc.Accept(writer);
		strRst = buffer.GetString();
	}

	// generate object value
	template<class TObject>
	static void GenerateObject(const TObject& dataObject, std::string& strRst)
	{
		rapidjson::Document doc(rapidjson::kObjectType);
		rapidjson::Document::AllocatorType& typeAllocate = doc.GetAllocator();
		doc.AddMember("code", 0, typeAllocate);
		// info
		{
			rapidjson::Value valueInfo(rapidjson::kObjectType);
			to_json(dataObject, valueInfo, typeAllocate);

			doc.AddMember("info", valueInfo, typeAllocate);
		}
		doc.AddMember("msg", rapidjson::Value("", typeAllocate), typeAllocate);

		rapidjson::StringBuffer buffer;
		rapidjson::Writer<rapidjson::StringBuffer>  writer(buffer);
		doc.Accept(writer);
		strRst = buffer.GetString();
	}

	///*
	//获取选定属性的列表 ref： https://www.likecs.com/ask-7300511.html
	//用法：
	//const rapidjson::Value& attributes = config["attributes"];
	//assert(attributes.IsArray());
	//std::vector<std::string> keysToRetrieve = { "maximumValue", "minimumValue" };
	//std::map<std::string, std::string> mapForResult = mapForAttributeThatMatchesName(attributes, "name", "mass", keysToRetrieve);
	//for (auto& mapItem : mapForResult) {

	//	std::cout << mapItem.first << ":" << mapItem.second << "\n";
	//}
	//*/
	//template<class TList>
	//static std::map<std::string, std::string> mapForAttributeThatMatchesName(const rapidjson::Value& attributes, const std::string& findMemberName, const std::string& findMemberValue, const std::vector<std::string>& keysToRetrieve) {

	//	std::map<std::string, std::string> result;
	//	for (rapidjson::Value::ConstValueIterator itr = attributes.Begin(); itr != attributes.End(); ++itr) {

	//		const rapidjson::Value::ConstMemberIterator currentAttribute = itr->FindMember(findMemberName.c_str());
	//		if (currentAttribute != itr->MemberEnd() && currentAttribute->value.IsString()) {

	//			if (currentAttribute->value == findMemberValue.c_str()) {

	//				for (auto& keyToRetrieve : keysToRetrieve) {

	//					const rapidjson::Value::ConstMemberIterator currentAttributeToReturn = itr->FindMember(keyToRetrieve.c_str());
	//					if (currentAttributeToReturn != itr->MemberEnd() && currentAttributeToReturn->value.IsString()) {

	//						result[keyToRetrieve] = currentAttributeToReturn->value.GetString();
	//					}
	//				}
	//				return result;
	//			}
	//		}
	//	}
	//	return result;
	//}

	//template<class TList>
	//static std::map<std::string, std::string> map_list_matches_names(const rapidjson::Value& attributes, const std::vector<std::string>& keysToRetrieve) {

	//	std::map<std::string, std::string> result;
	//	for (rapidjson::Value::ConstValueIterator itr = attributes.Begin(); itr != attributes.End(); ++itr) {

	//		for (auto& keyToRetrieve : keysToRetrieve) {

	//			const rapidjson::Value::ConstMemberIterator currentAttributeToReturn = itr->FindMember(keyToRetrieve.c_str());
	//			if (currentAttributeToReturn != itr->MemberEnd() && currentAttributeToReturn->value.IsString()) {

	//				result[keyToRetrieve] = currentAttributeToReturn->value.GetString();
	//			}
	//		}
	//		return result;
	//	}
	//	return result;
	//}
}