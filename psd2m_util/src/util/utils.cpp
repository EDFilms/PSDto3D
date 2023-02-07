//----------------------------------------------------------------------------------------------
//
//  @file utils.cpp
//  @author Michaelson Britt
//  @date 21-NOV-2021
//
//  @section DESCRIPTION
//
//----------------------------------------------------------------------------------------------

#include <Windows.h> // for OutputDebugString(), MultiByteToWideChar(), and wrapper functions
#include <tchar.h>
#include <map>
#include "utils.h"

namespace util
{
	void DebugPrint(const char* lpszFormat, ...)
	{
		const TCHAR* format = lpszFormat;
		va_list args;
		va_start(args, lpszFormat);
		int nBuf;
		TCHAR szBuffer[512]; // get rid of this hard-coded buffer
		nBuf = _vsnprintf_s(szBuffer, 511, format, args);
		::OutputDebugString(szBuffer);
		va_end(args);
	}

	//----------------------------------------------------------------------------------------
	// Windows wrappers
	unsigned long Windows_GetCurrentThreadId()
	{
		return GetCurrentThreadId();
	}

	long long Windows_InterlockedCompareExchange64( long long volatile *Destination, long long ExChange, long long Comperand )
	{
		return InterlockedCompareExchange64( Destination, ExChange, Comperand );
	}




	//----------------------------------------------------------------------------------------
	// String helpers

	class StringTable::StringTableMap : public std::map< std::pair<int,int>, const char* >
	{
	public:
		bool isEnglish;
	};

	bool StringTable::IsEnglish()
	{
		return ((stringTableMap==nullptr) || (stringTableMap->isEnglish));
	}

	// last item should be bookend with id of -1
	void StringTable::AddStrings( int context, const StringTableItem* items, bool isEnglish )
	{
		if( stringTableMap==nullptr )
			stringTableMap = new StringTableMap();

		for( const StringTableItem* item=items; item->id>=0; item++ )
			(*stringTableMap)[ std::pair<int,int>(context,item->id) ] = item->str;

		stringTableMap->isEnglish = isEnglish;
	}

	const char* StringTable::Lookup( int context, int id )
	{
		StringTableMap::iterator iter = stringTableMap->find( std::pair<int,int>(context,id) );
		if( iter!=stringTableMap->end() )
			return iter->second;
		const char* notFound = "== TODO: DEFINE THIS ==";
		return notFound;
	}

	bool IsLocalizationEnglish()
	{
		return (GetLocalizationStringTable()->IsEnglish());
	}
	StringTable* GetLocalizationStringTable()
	{
		static StringTable translator;
		return &translator;
	}
	const char* LocalizeString( int context, int id )
	{
		return GetLocalizationStringTable()->Lookup( context, id );
	}
	const std::string LocalizeStringUTF8( int context, int id )
	{
		const char* result = LocalizeString(context,id);
		if( result==nullptr )
			return std::string();
		return std::string( result );
	}
	const std::wstring LocalizeStringUTF16( int context, int id )
	{
		const char* result = LocalizeString(context,id);
		if( result==nullptr )
			return std::wstring();
		return to_utf16( result, (int)(strlen(result)) );
	}

	//----------------------------------------------------------------------------------------------
	std::wstring to_utf16( const char* string_utf8, int length_utf8 )
	{
		int length_utf16 = ::MultiByteToWideChar(
			CP_UTF8,
			0,
			string_utf8,
			length_utf8,
			NULL,
			0);
		if (length_utf16 == 0) return L"";

		std::wstring string_utf16;
		string_utf16.resize(length_utf16);
		::MultiByteToWideChar(
			CP_UTF8,
			0,
			string_utf8,
			length_utf8,
			const_cast< wchar_t* >(string_utf16.c_str()),
			length_utf16); 

		return string_utf16;
	}

	//----------------------------------------------------------------------------------------------
	std::wstring to_utf16( const std::string& string_utf8 )
	{
		return to_utf16(string_utf8.c_str(), (int)string_utf8.size());
	}

}
