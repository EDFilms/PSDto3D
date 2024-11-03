//----------------------------------------------------------------------------------------------
// ===============================================
//  Copyright (C) 2016, CDRIN.
//  All Rights Reserved.
// ===============================================
//  Unauthorized copying of this file, via any medium is strictly prohibited
//  Proprietary and confidential
//
//  @file PsdReader.cpp
//  @author Fabien Raspail
//  @date 11-11-2016
//
//  @section DESCRIPTION
//
//----------------------------------------------------------------------------------------------


#include "psdReader.h"
#include "util/utils.h"
#include "util/progressJob.h"

namespace psd_reader
{

#pragma region CONSRUCTOR

	//----------------------------------------------------------------------------------------
	PsdReader::PsdReader(const char* filename_utf8)
	{
		if (!DoesFileExist(filename_utf8))
			return;

		this->File = _wfsopen( util::to_utf16(filename_utf8).c_str(), L"rb", _SH_DENYWR);
	}

	//----------------------------------------------------------------------------------------
	PsdReader::~PsdReader() = default;

#pragma endregion 

	//----------------------------------------------------------------------------------------
	void PsdReader::SetProgress(util::ProgressJob* progressJob)
	{
		this->ProgressJob = progressJob;
	}

	//----------------------------------------------------------------------------------------
	bool PsdReader::DoesFileExist(const char *filename_utf8)
	{
		std::ifstream file( util::to_utf16(filename_utf8) );
		const auto fileExist = file.good();
		std::cout << (fileExist ? "[PARSING PATH] It is a valid Path." : "[PARSING PATH] It is not a valid Path.") << std::endl;
		return fileExist;
	}

#pragma region LOAD

	//----------------------------------------------------------------------------------------
	PsdData PsdReader::ParsePsd()
	{
		PsdData data;

		ParseSection(data);
		std::cout << "Parsing PSD complete." << std::endl;
		fclose(this->File);
		return data;
	}

	//----------------------------------------------------------------------------------------
	void PsdReader::ParseSection(PsdData& data) const
	{
		if (this->File == nullptr) return;

		if( (!LoadHeader(data))         ||  (this->ProgressJob->IsCancelled()) ) return;
		if( (!LoadColorModeData(data))  ||  (this->ProgressJob->IsCancelled()) ) return;
		if( (!LoadImageResource(data))  ||  (this->ProgressJob->IsCancelled()) ) return;
		if( (!LoadLayerAndMask(data))   ||  (this->ProgressJob->IsCancelled()) ) return;
		// Use to read the complete merged/composite image
		// Not implemented
		//if (!LoadImageData(data)) return;
	}

	//----------------------------------------------------------------------------------------
	bool PsdReader::LoadHeader(PsdData & data) const
	{
		bool success;	// No errors
		try
		{
			success = HeaderReader::Read(this->File, data.HeaderData);
			if (!success)
			{
				std::cout << "[PARSING HEADER] Error parsing Header" << std::endl;
			}
		}
		catch (...)
		{
			success = false;
		}

		return success;
	}

	//----------------------------------------------------------------------------------------
	bool PsdReader::LoadColorModeData(PsdData & data) const
	{
		bool success;	// No errors
		try
		{
			success = ColorModeReader::Read(this->File, data.ColorModeData);
			if (!success)
			{
				std::cout << "[PARSING COLOR MODE] Error parsing Color Mode" << std::endl;
			}
		}
		catch (...)
		{
			success = false;
		}

		return success;
	}

	//----------------------------------------------------------------------------------------
	bool PsdReader::LoadImageResource(PsdData & data) const
	{
		bool success;	// No errors
		try
		{
			success = ImageResourceReader::Read(this->File, data.ImageResourceData);
			if (!success)
			{
				std::cout << "[PARSING IMAGE RESOURCE] Error parsing Image resource" << std::endl;
			}
		}
		catch (...)
		{
			success = false;
		}

		return success;
	}

	//----------------------------------------------------------------------------------------
	bool PsdReader::LoadLayerAndMask(PsdData & data) const
	{
		bool success;	// No errors
		try
		{
			success = LayerAndMaskReader::Read(this->File, data.HeaderData, data.LayerMaskData, *(this->ProgressJob));
			if (!success)
			{
				std::cout << "[PARSING LAYER AND MASK] Error parsing Layer Mask data" << std::endl;
			}
		}
		catch (...)
		{
			success = false;
		}

		return success;
	}

	//----------------------------------------------------------------------------------------
	bool PsdReader::LoadImageData(PsdData & data) const
	{
		bool success;	// No errors
		try
		{
			success = ImageDataReader::Read(this->File, data.ImageData, data.ImageResourceData.ResolutionInfo, data.HeaderData);
			if (!success)
			{
				std::cout << "[PARSING IMAGE DATA] Error parsing Image data" << std::endl;
			}
		}
		catch (...)
		{
			success = false;
		}

		return success;
	}
}
#pragma endregion