//----------------------------------------------------------------------------------------------
// ===============================================
//  Copyright (C) 2016, CDRIN.
//  All Rights Reserved.
// ===============================================
//  Unauthorized copying of this file, via any medium is strictly prohibited
//  Proprietary and confidential
//
//  @file Utils.h
//  @author Fabien Raspail
//  @date 11-11-2016
//
//  @section DESCRIPTION
//
//----------------------------------------------------------------------------------------------

#ifndef UTILS_H
#define UTILS_H

#include <string>

namespace util
{
	void DebugPrint(char* lpszFormat, ...);

	//----------------------------------------------------------------------------------------
	// Windows wrappers
	// allows header files with inline code to avoid including windows.h
	unsigned long Windows_GetCurrentThreadId();
	long long Windows_InterlockedCompareExchange64 ( long long volatile *Destination, long long ExChange, long long Comperand );

	//----------------------------------------------------------------------------------------
	// String helpers

	class StringTable
	{
	public:
		class StringTableMap; // forward declaration
		struct StringTableItem
		{
			int id;
			const char* str; // UTF8
		};

		StringTable() : stringTableMap(nullptr) {}
		bool IsEnglish();

		void AddStrings( int context, const StringTableItem* items, bool isEnglish ); // last item should be bookend with id of -1
		const char* Lookup( int context, int id );

	protected:
		StringTableMap* stringTableMap;
	};

	// Language translation
	bool IsLocalizationEnglish();
	StringTable* GetLocalizationStringTable();
	const char* LocalizeString( int context, int id );
	const std::string LocalizeStringUTF8( int context, int id );
	const std::wstring LocalizeStringUTF16( int context, int id );

	//----------------------------------------------------------------------------------------------
	// reinterpret a sequence of character bytes as an integer
	// assumes digits is four or less (meaning, digits <= sizeof(int))
	inline int StringAsInt(unsigned char* c, int digits)
	{
		int value = 0;
		for (int n = 0; n < digits; ++n)
		{
			value = (value << 8) | *(c + n);
		}
		return value;
	};

	//----------------------------------------------------------------------------------------------
	inline bool CheckSignature(const char* osType, const std::string& signature)
	{
		const std::string strOsType = std::string(osType, 4);
		return (signature == strOsType);
	}

	//----------------------------------------------------------------------------------------------
	inline std::string PascalString(int size, unsigned char* value)
	{
		std::string stringContent;
		for(auto i=0; i<size; i++)
		{
			int cInt = StringAsInt(&value[i], sizeof(unsigned char));
			const auto character = static_cast<char>(cInt);
			stringContent += character;
		}
		return stringContent;
	}

	//----------------------------------------------------------------------------------------------
	inline void ReplaceBackslashes( wchar_t* str, int len )
	{
		wchar_t bslash=wchar_t('\\'), fslash=wchar_t('/'); // TODO: is this TCHAR macro correct?
		for( int i=0; (i<len) && ((*str)!=wchar_t(0)); i++, str++ ) (*str) = ((*str)==bslash? fslash:(*str));
	}

	//----------------------------------------------------------------------------------------------
	inline void ReplaceForwardslashes( wchar_t* str, int len )
	{
		wchar_t bslash=wchar_t('\\'), fslash=wchar_t('/'); // TODO: is this TCHAR macro correct?
		for( int i=0; (i<len) && ((*str)!=wchar_t(0)); i++, str++ ) (*str) = ((*str)==fslash? bslash:(*str));
	}

	//----------------------------------------------------------------------------------------------
	inline void ReplaceWhitespace( wchar_t* str, int len )
	{
		wchar_t bslash=wchar_t('\\'), fslash=wchar_t('/'); // TODO: is this TCHAR macro correct?
		for( int i=0; (i<len) && ((*str)!=wchar_t(0)); i++, str++ ) (*str) = ((*str)==fslash? bslash:(*str));
	}

	//----------------------------------------------------------------------------------------------
	std::wstring to_utf16( const char* string_utf8, int length_utf8 );

	//----------------------------------------------------------------------------------------------
	std::wstring to_utf16( const std::string& string_utf8 );

	//----------------------------------------------------------------------------------------------
	inline bool IsLegalFilenameChar( char c )
	{
		// windows doesn't allow /, \, ?, %, *, :, |, ", <, >
		// ASCII value 32 and less are space, tab and control characters
		// ASCII value 127 is delete
		if( (c<=32) || (c==127) || (c=='<') || (c=='>') || (c==':') ||
			(c=='\"') || (c=='/') || (c=='|') || (c=='?') || (c=='*') )
			return false;
		return true;
	}

	//----------------------------------------------------------------------------------------------
	inline const std::string NormalizeFilenameString( const std::string string_in )
	{
		std::string string_out;
		string_out.resize( string_in.size() );
		for( int i=0; i<string_in.size(); i++ )
		{
			char c = string_in[i];
			string_out[i] = (IsLegalFilenameChar(c)? c : '_');
		}
		return string_out;
	}


}
#endif // UTILS_H
