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

namespace psd_reader
{

#pragma region CONSRUCTOR

	//----------------------------------------------------------------------------------------
	PsdReader::PsdReader(std::string const& pathFile)
	{
		const char* cstrFileName(pathFile.c_str());
		if (!DoesFileExist(cstrFileName))
			return;

		this->File = _fsopen(cstrFileName, "rb", _SH_DENYWR);
	}

	//----------------------------------------------------------------------------------------
	PsdReader::~PsdReader() = default;

#pragma endregion 

	//----------------------------------------------------------------------------------------
	void PsdReader::SetProgress(std::function<void(unsigned)> & initializeProgress, std::function<void(unsigned)>& initializeSubProgress
		, std::function<void()> & incrementProgress, std::function<void()> & completeSubProgress)
	{
		this->ProgressData = PsdProgress(initializeProgress, initializeSubProgress, incrementProgress, completeSubProgress);
	}

	//----------------------------------------------------------------------------------------
	bool PsdReader::DoesFileExist(const char *filename)
	{
		std::ifstream file(filename);
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

		if (!LoadHeader(data)) return;
		if (!LoadColorModeData(data)) return;
		if (!LoadImageResource(data)) return;
		if (!LoadLayerAndMask(data)) return;
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
			success = LayerAndMaskReader::Read(this->File, data.HeaderData, data.LayerMaskData, this->ProgressData);
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