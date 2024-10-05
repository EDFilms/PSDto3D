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
#include <vector>
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

	//----------------------------------------------------------------------------------------
	class StringTable::StringTableMap : public std::map< std::pair<int,int>, const char* >
	{
	public:
		bool isEnglish;
	};

	StringTable::StringTable()
	{
		stringTableMap = new StringTableMap();
	}

	StringTable::~StringTable()
	{
		SafeDelete( stringTableMap );
	}

	bool StringTable::IsEnglish()
	{
		return ((stringTableMap==nullptr) || (stringTableMap->isEnglish));
	}

	// last item should be bookend with id of -1
	void StringTable::AddStrings( int context, const StringTableItem* items, bool isEnglish )
	{
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

	//----------------------------------------------------------------------------------------
	typedef std::pair<std::string,int> PairStringInt;
	class NameToNameMap::NameTableMap : public std::vector< PairStringInt >
	{
	public:
		int Lookup( int i ) { return this->operator[](i).second; }
		// call this once for each "source" name, in sequential order, to be stored in the table
		void AddSrc( const char* srcName, int index=-1 )
		{
			this->push_back( PairStringInt(srcName,index) );
		}
		// call this once for each "mapped" name, in sequential order, which table items are mapped to;
		// finds first item where dstName matches, but has index -1, updating it to the given index
		int AddDst( const char* dstName, int index )
		{
			for( int i=0; i<size(); i++ )
			{
				PairStringInt& item = this->operator[](i);
				if( (item.second==-1) && (item.first==dstName) )
				{
					item.second = index;
					return i;
				}
			}
			return -1;
		}
	};

	NameToNameMap::NameToNameMap()
	{
		srcNameMap = new NameTableMap();
		dstNameMap = new NameTableMap();
	}

	NameToNameMap::~NameToNameMap()
	{
		SafeDelete( srcNameMap );
		SafeDelete( dstNameMap );
	}

	void NameToNameMap::InitSrcName( const char* str )
	{
		srcNameMap->AddSrc( str );
	}

	void NameToNameMap::InitDstName( const char* str )
	{
		int dstIndex = (int)dstNameMap->size();
		int srcIndex = srcNameMap->AddDst( str, dstIndex );
		dstNameMap->AddSrc( str, srcIndex );
	}

	int NameToNameMap::MapSrcToDst( int index )
	{
		return srcNameMap->Lookup(index);
	}

	int NameToNameMap::MapDstToSrc( int index )
	{
		return dstNameMap->Lookup(index);
	}


	//----------------------------------------------------------------------------------------
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
