//----------------------------------------------------------------------------------------------
// ===============================================
//  Copyright (C) 2016, CDRIN.
//  All Rights Reserved.
// ===============================================
//  Unauthorized copying of this file, via any medium is strictly prohibited
//  Proprietary and confidential
//
//  @file ImageDataReader.cpp
//  @author Fabien Raspail
//  @date 11-11-2016
//
//  @section DESCRIPTION
//
//----------------------------------------------------------------------------------------------

#include <iostream>
#include "imageDataReader.h"
#include "util/utils.h"

namespace psd_reader
{
	ImageData::ImageData()
	{
		Data = nullptr;
	} 
	
	//----------------------------------------------------------------------------------------
	ImageData::~ImageData()
	{
		delete Data;
	}


	//----------------------------------------------------------------------------------------
	bool ImageDataReader::Read(FILE* file, ImageData& combinedImage, ResolutionInfo const& resInfo, HeaderData const& headerInfo)
	{
		// Not implemented In the project


		/*if (!feof(file))
		{
			// Read Compression method
			unsigned char compValue[2];
			fread(&compValue, sizeof(compValue), 1, file);
			const auto compression = short(util::Utils::Calculate(compValue, sizeof(compValue)));

			switch (compression)
			{
				// raw data
			case 0:
			{
				std::cout << "[ImageDataReader] Raw Data" << std::endl;
				RawDataReader(file, combinedImage, resInfo, headerInfo);
			}
			break;

			// RLE compression
			case 1:
			{
				std::cout << "[ImageDataReader] RLE compression" << std::endl;
				RleCompressionReader(file, combinedImage, resInfo, headerInfo);
			}
			break;

			// ZIP without prediction
			case 2:
			{
				std::cout << "[ImageDataReader] ZIP without prediction, not implemented" << std::endl;
			}
			break;

			// ZIP with prediction
			case 3:
			{
				std::cout << "[ImageDataReader] ZIP with prediction, not implemented" << std::endl;
			}
			break;

			default:
			{
				std::cout << "[ImageDataReader] No compression found" << std::endl;
			}

			}
		}*/
		return  true;
	}


	//----------------------------------------------------------------------------------------
	void ImageDataReader::RleCompressionReader(FILE* file, ImageData& combinedImage, ResolutionInfo const& resInfo, HeaderData const& headerInfo)
	{
		const int headerWidth = headerInfo.Width;
		const int headerHeight = headerInfo.Height;
		const int bytesPerPixelPerChannel = headerInfo.BitsPerPixel / 8;

		const int pixels = headerWidth * headerHeight;
		const int totalBytes = pixels * bytesPerPixelPerChannel * headerInfo.Channels;

		auto* dataImage = new unsigned char[totalBytes];
		memset(dataImage, 254, totalBytes);

		unsigned char* p = dataImage;
		int value;

		unsigned char byteValue[1];

		// The RLE-compressed data is preceded by a 2-byte data count for each row in the data,
		// which we're going to just skip.
		fseek(file, headerHeight * headerInfo.Channels * 2, SEEK_CUR);

		for (int channel = 0; channel < headerInfo.Channels; channel++)
		{
			// Read the RLE data.
			int count = 0;
			while (count < pixels)
			{
				fread(&byteValue, sizeof(byteValue), 1, file);

				int len = util::Utils::Calculate(byteValue, sizeof(byteValue));
				if (128 > len)
				{
					len++;
					count += len;

					while (len)
					{
						fread(&byteValue, sizeof(byteValue), 1, file);

						value = util::Utils::Calculate(byteValue, sizeof(byteValue));

						*p = value;
						p += sizeof(byteValue);
						len--;
					}
				}
				else if (128 < len)
				{
					// Next -len+1 bytes in the dest are replicated from next source byte.
					// (Interpret len as a negative 8-bit int.)
					len ^= 0x0FF;
					len += 2;
					fread(&byteValue, sizeof(byteValue), 1, file);

					value = util::Utils::Calculate(byteValue, sizeof(byteValue));

					count += len;

					while (len)
					{
						*p = value;
						p += sizeof(byteValue);
						len--;
					}
				}
				else if (128 == len)
				{
					// Do nothing
				}
			}
		}

		unsigned char* source = dataImage;
		auto* dest = new unsigned char[totalBytes];
		memset(dest, 254, totalBytes);

		for (int colour = 0; colour < headerInfo.Channels; ++colour)
		{
			int pixelCounter = colour * bytesPerPixelPerChannel;
			for (int nPos = 0; nPos < pixels; ++nPos)
			{
				memcpy(dest + pixelCounter, source, bytesPerPixelPerChannel);
				source++;

				pixelCounter += headerInfo.Channels*bytesPerPixelPerChannel;
			}
		}

		delete[] dataImage;
		dataImage = dest;

		if (dataImage)
		{
			//const auto nHorResolution = int(resInfo.HRes);
			//const auto nVertResolution = int(resInfo.VRes);
			//int ppmX = (nHorResolution * 10000) / 254;
			//int ppmY = (nVertResolution * 10000) / 254;

			switch (headerInfo.BitsPerPixel)
			{
			case 1:
			{
				std::cout << "Not implemented" << std::endl; // Not implemented
			}
			break;

			case 8:
			case 16:
			{
				//CreateDIBSection(nWidth, nHeight, ppm_x, ppm_y, 24);
				//break;
			}
			break;

			default: ;
			}
		}

		delete[] dataImage;
		dataImage = nullptr;
	}


	//----------------------------------------------------------------------------------------
	void ImageDataReader::RawDataReader(FILE* file, ImageData& combinedImage, ResolutionInfo const& resInfo, HeaderData const& headerInfo)
	{
		const int width = headerInfo.Width;
		const int height = headerInfo.Height;
		const int bytesPerPixelPerChannel = headerInfo.BitsPerPixel / 8;

		const int pixels = width * height;
		int totalBytes = pixels * bytesPerPixelPerChannel * headerInfo.Channels;

		unsigned char* proccessBuffer = nullptr;
		unsigned char* dataImage;

		int bytesRead = 0;
		switch (headerInfo.ColourMode)
		{
		case 1:		// Grayscale
		case 8:		// Duotone
		{
			proccessBuffer = new unsigned char[totalBytes];
			dataImage = new unsigned char[bytesPerPixelPerChannel];
			memset(proccessBuffer, 254, totalBytes);
			memset(dataImage, 254, bytesPerPixelPerChannel);

			while (!feof(file) && (bytesRead < totalBytes))
			{
				memset(dataImage, 254, bytesPerPixelPerChannel);
				fread(dataImage, bytesPerPixelPerChannel, 1, file);
				memcpy(proccessBuffer + bytesRead, dataImage, bytesPerPixelPerChannel);
				bytesRead += bytesPerPixelPerChannel;
			}

			delete[] dataImage;
			dataImage = nullptr;
		}
		break;
		case 2:		// Indexed
		{
			proccessBuffer = new unsigned char[totalBytes];
			dataImage = new unsigned char[bytesPerPixelPerChannel];
			memset(proccessBuffer, 254, totalBytes);
			memset(dataImage, 254, bytesPerPixelPerChannel);

			while (!feof(file) && (bytesRead < totalBytes))
			{
				memset(dataImage, 254, bytesPerPixelPerChannel);
				fread(dataImage, bytesPerPixelPerChannel, 1, file);
				memcpy(proccessBuffer + bytesRead, dataImage, bytesPerPixelPerChannel);
				bytesRead += bytesPerPixelPerChannel;
			}

			delete[] dataImage;
			dataImage = nullptr;
		}
		break;
		case 3:		// RGB
		{
			int bytesToReadPerPixelPerChannel = bytesPerPixelPerChannel;
			if (2 == bytesToReadPerPixelPerChannel)
			{
				bytesToReadPerPixelPerChannel = 1;
				totalBytes = pixels * bytesToReadPerPixelPerChannel * headerInfo.Channels;
			}

			dataImage = new unsigned char[bytesToReadPerPixelPerChannel];
			proccessBuffer = new unsigned char[totalBytes];
			memset(proccessBuffer, 254, totalBytes);
			memset(dataImage, 254, bytesToReadPerPixelPerChannel);

			for (int nColour = 0; nColour < 3; ++nColour)
			{
				int pixelCounter = nColour;
				for (int nPos = 0; nPos < pixels; ++nPos)
				{
					if (!feof(file)) continue;
					
					fread(dataImage, bytesToReadPerPixelPerChannel, 1, file);
					memcpy(proccessBuffer + pixelCounter, dataImage, bytesToReadPerPixelPerChannel);
					bytesRead += bytesToReadPerPixelPerChannel;
					pixelCounter += 3;

					if (2 == bytesPerPixelPerChannel) continue;

					fread(dataImage, bytesToReadPerPixelPerChannel, 1, file);
				}
			}

			delete[] dataImage;
			dataImage = nullptr;
		}
		break;
		case 4:	// CMYK
		{
			dataImage = new unsigned char[bytesPerPixelPerChannel];
			proccessBuffer = new unsigned char[totalBytes];
			memset(proccessBuffer, 254, totalBytes);
			memset(dataImage, 254, bytesPerPixelPerChannel);

			for (int nColour = 0; nColour < headerInfo.Channels; ++nColour)
			{
				int pixelCounter = nColour * bytesPerPixelPerChannel;
				for (int nPos = 0; nPos < pixels; ++nPos)
				{
					if (!feof(file))
					{
						fread(dataImage, bytesPerPixelPerChannel, 1, file);
						memcpy(proccessBuffer + pixelCounter, dataImage, bytesPerPixelPerChannel);
						bytesRead += bytesPerPixelPerChannel;

						pixelCounter += headerInfo.Channels*bytesPerPixelPerChannel;
					}
				}
			}

			delete[] dataImage;
			dataImage = nullptr;
		}
		break;
		case 9:	// Lab
		{
			dataImage = new unsigned char[bytesPerPixelPerChannel];
			proccessBuffer = new unsigned char[totalBytes];
			memset(proccessBuffer, 254, totalBytes);
			memset(dataImage, 254, bytesPerPixelPerChannel);

			for (int colour = 0; colour < 3; ++colour)
			{
				int pixelCounter = colour * bytesPerPixelPerChannel;
				for (int pos = 0; pos < pixels; ++pos)
				{
					if (!feof(file))
					{
						fread(dataImage, bytesPerPixelPerChannel, 1, file);
						memcpy(proccessBuffer + pixelCounter, dataImage, bytesPerPixelPerChannel);
						bytesRead += bytesPerPixelPerChannel;
						pixelCounter += 3 * bytesPerPixelPerChannel;
					}
				}
			}

			delete[] dataImage;
			dataImage = nullptr;
		}
		break;

		default: ;
		}

		if (bytesRead == totalBytes && proccessBuffer != nullptr)
		{		
			//int nHorResolution = int(resInfo.HRes);
			//int nVertResolution = int(resInfo.VRes);
			//int ppm_x = (nHorResolution * 10000) / 254;
			//int ppm_y = (nVertResolution * 10000) / 254;

			switch (headerInfo.BitsPerPixel)
			{
			case 1:
			{
				// Not yet implemented
			}
			break;
			case 8:
			case 16:
			{
				// CreateDIBSection(nWidth, nHeight, ppm_x, ppm_y, 24);
				//break;
			}
			break;
			default:
			{
				// Unsupported format
			}
			}

			/*HBITMAP hBitmap = mhBitmap;

			if (!hBitmap)
			{
				delete[] ProccessBuffer;
				ProccessBuffer = 0;

				delete[] dataImage;
				dataImage = 0;

				nErrorCode = -9;	// Cannot create hBitmap
				return nErrorCode;
			}
			ProccessBuffer(ProccessBuffer);*/			
		}

		delete[] proccessBuffer;
		proccessBuffer = nullptr;
	}
}
