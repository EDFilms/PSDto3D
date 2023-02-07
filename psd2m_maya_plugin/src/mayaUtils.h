//----------------------------------------------------------------------------------------------
// ===============================================
//  Copyright (C) 2016, CDRIN.
//  All Rights Reserved.
// ===============================================
//  Unauthorized copying of this file, via any medium is strictly prohibited
//  Proprietary and confidential
//
//  @file pluginController.h
//  @author Fabien Raspail
//  @date 11-11-2016
//
//  @section DESCRIPTION
//
//----------------------------------------------------------------------------------------------

#ifndef MAYAUTILS_H
#define MAYAUTILS_H

#include <string>

namespace maya_plugin
{
	class MayaUtils
	{
	public:
		static inline bool IsSpecialChar( char c )
		{
			// Maya forbids everything except a-zA-Z0-9 and underscore
			return !(isalnum(c) || (c=='_'));
		}
		// Convert special characters to underscore
		static inline std::string NormalizeName(const std::string name_in)
		{
			std::string name_out;
			int idx_out=0, len_in=(int)name_in.length();
			bool run=false;
			name_out.resize(len_in+2);
			// If the first character is not alphabetic a-zA-Z, then prepend an underscore
			if (!isalpha(name_in[0]))
			{
				name_out[0]='_';
				idx_out++;
			}
			// Convert special characters to underscore
			for( int idx_in=0; idx_in<len_in; idx_in++ )
			{
				char c = name_in[idx_in];
				// copy character if it's alphanumeric a-zA-Z0-9
				if( isalnum(c) )
				{
					name_out[idx_out] = c;
					idx_out++;
					run=false;
				}
				// copy an underscore otherwise
				else
				{
					name_out[idx_out] = '_';
					// increment position for first special character in a run of several
					idx_out += (run? 0:1);
					run = true;
				}
			}
			// Remove trailing underscore if any
			if( (idx_out>1) && (name_out[idx_out-1]=='_') )
			{
				idx_out--;
			}
			name_out.resize(idx_out);
			return name_out;
		}
	};
}
#endif // MAYAUTILS_H
