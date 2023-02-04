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
	//----------------------------------------------------------------------------------------------
	class Utils
	{
	public:
		//----------------------------------------------------------------------------------------------
		static int Calculate(unsigned char* c, int digits)
		{
			int value = 0;
			for (int n = 0; n < digits; ++n)
			{
				value = (value << 8) | *(c + n);
			}
			return value;
		};

		//----------------------------------------------------------------------------------------------
		static bool CheckSignature(const char* osType, const std::string& signature)
		{
			const std::string strOsType = std::string(osType, 4);
			return (signature == strOsType);
		}

		//----------------------------------------------------------------------------------------------
		static std::string PascalString(int size, unsigned char* value)
		{
			std::string stringContent;
			for(auto i=0; i<size; i++)
			{
				int cInt = Calculate(&value[i], sizeof(unsigned char));
				const auto character = static_cast<char>(cInt);
				stringContent += character;
			}
			return stringContent;
		}
	};
}
#endif // UTILS_H
