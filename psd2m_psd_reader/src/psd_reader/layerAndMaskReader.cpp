//----------------------------------------------------------------------------------------------
// ===============================================
//  Copyright (C) 2016, CDRIN.
//  All Rights Reserved.
// ===============================================
//  Unauthorized copying of this file, via any medium is strictly prohibited
//  Proprietary and confidential
//
//  @file LayerAndMaskReader.cpp
//  @author Fabien Raspail
//  @date 11-11-2016
//
//  @section DESCRIPTION
//
//----------------------------------------------------------------------------------------------

#include <string>
#include <locale>

#include "layerAndMaskReader.h"
#include "util/utils.h"
#include "imageResourceReader.h"
#include <algorithm>
#include <functional>
#include "zlib.h"
#include "headerReader.h"

using namespace util;

namespace psd_reader
{
	//----------------------------------------------------------------------------------------
	const std::string LayerAndMaskData::INFLUENCE_LAYER_TAG("influence");
	const int LayerAndMaskReader::PATH_BLOCK_SIZE(26);
	

	//----------------------------------------------------------------------------------------
	std::string LayerAndMaskData::LayerNameInfluenceAssociated(std::string const& str)
	{
		std::string layerName;
		std::string name = str;

		std::transform(name.begin(), name.end(), name.begin(), tolower);

		const auto found = name.find_last_of('_');
		if (name.substr(found + 1) == (INFLUENCE_LAYER_TAG))
		{
			layerName = str.substr(0, found);
		}
		return layerName;
	}

	//----------------------------------------------------------------------------------------
	static bool CheckSignatureLayerInfo(char* const signature)
	{
		const std::string strSignature(signature, 4);
		return ("8BIM" == strSignature || "8B64" == strSignature);
	}

	//----------------------------------------------------------------------------------------
	bool LayerAndMaskReader::Read(FILE* file, const HeaderData& headerData, LayerAndMaskData& layerMaskData, ProgressJob& progress)	// Actually ignore it
	{
		unsigned char dataLength[4];
		int bytesRead = 0;

		fread(&dataLength, sizeof(dataLength), 1, file);
		bytesRead += sizeof(dataLength);
		const int totalBytes = util::StringAsInt(dataLength, sizeof(dataLength));

		ProcessLayerMaskInformation(file, bytesRead, headerData, layerMaskData, progress);
		ReadHeaderAdditionalLayerGlobalInfo(file, bytesRead, headerData, layerMaskData, progress);

		const int extradata = totalBytes - bytesRead;
		if (extradata > 0)
		{
			const auto data = new unsigned char[extradata];
			fread(data, sizeof(unsigned char) * extradata, 1, file);
			bytesRead += (sizeof(unsigned char) * extradata);
		}

		return (bytesRead == totalBytes);
	}


	//----------------------------------------------------------------------------------------
	void LayerAndMaskReader::ProcessLayerMaskInformation(FILE* file, int& bytesRead, const HeaderData& headerData, LayerAndMaskData& layerMaskData, ProgressJob& progress)
	{
		ReadLayerInfoSection(file, bytesRead, headerData,layerMaskData, progress);
		ReadGlobalLayerMaskInfo(file, bytesRead, layerMaskData);

		if (!layerMaskData.Layers.empty())
		{
			std::reverse(layerMaskData.Layers.begin(), layerMaskData.Layers.end());
		}
	}

	//----------------------------------------------------------------------------------------
	bool LayerAndMaskReader::ReadLayerInfoSection(FILE* file, int& bytesRead, const HeaderData& headerData, LayerAndMaskData& layerMaskData, ProgressJob& progress)
	{
		unsigned char dataLength[4];
		unsigned char layerLength[2];
		int bytesLayerinfoRead = 0;

		fread(&dataLength, sizeof(dataLength), 1, file);
		bytesRead += sizeof(dataLength);
		int totalBytesLayer = util::StringAsInt(dataLength, sizeof(dataLength));
		totalBytesLayer += totalBytesLayer % 2;

		if (totalBytesLayer == 0) return true;

		//Layer count. If it is a negative number, its absolute value is the number of layers and the first alpha channel contains the transparency data for the merged result.
		fread(&layerLength, sizeof(layerLength), 1, file);
		bytesLayerinfoRead += sizeof(layerLength);
		// If it is a negative number, its absolute value is the number of layers and the first alpha channel contains the transparency data for the merged result.
		const short layerCount = std::abs(short(util::StringAsInt(layerLength, sizeof(layerLength))));

		for( short layerIndex = 0; layerIndex < layerCount; layerIndex++ )
		{
			LayerData currentlayer;
			currentlayer.LayerIndex = layerIndex;
			ReadLayerRecordsSection(file, bytesLayerinfoRead, currentlayer);
			ReadLayerInfoSectionExtraDataField(file, bytesLayerinfoRead, currentlayer);
			if(currentlayer.Type == TEXTURE_LAYER)
			{
				std::string layerName = LayerAndMaskData::LayerNameInfluenceAssociated(currentlayer.LayerName);
				if (!layerName.empty())
				{
					currentlayer.Type = INFLUENCE_LAYER;
				}
			}
			layerMaskData.Layers.push_back(currentlayer);
		}

		ReadChannelImageData(file, bytesLayerinfoRead, headerData.BitsPerPixel, layerMaskData, progress);

		// Read the extra size 
		const int extradata = totalBytesLayer - bytesLayerinfoRead;
		if (extradata > 0)
		{
			auto* data = new unsigned char[extradata];
			fread(data, sizeof(unsigned char) * extradata, 1, file);
			bytesLayerinfoRead += (sizeof(unsigned char) * extradata);
		}

		bytesRead += bytesLayerinfoRead;
		return totalBytesLayer == bytesLayerinfoRead;
	}

	//----------------------------------------------------------------------------------------
	bool LayerAndMaskReader::ReadLayerRecordsSection(FILE* file, int& bytesRead, LayerData& layerData)
	{
		int tmpBytesRead = 0;

		unsigned char dataLength[4];
		unsigned char coordinateLayer[4];
		unsigned char channelCount[2];

		// Rectangle containing the contents of the layer. Specified as top, left, bottom, right coordinates
		fread(&coordinateLayer, sizeof(coordinateLayer), 1, file); // Top
		layerData.AnchorTop = util::StringAsInt(coordinateLayer, sizeof(coordinateLayer));

		fread(&coordinateLayer, sizeof(coordinateLayer), 1, file); //Left
		layerData.AnchorLeft = util::StringAsInt(coordinateLayer, sizeof(coordinateLayer));

		fread(&coordinateLayer, sizeof(coordinateLayer), 1, file); // Bottom
		layerData.AnchorBottom = util::StringAsInt(coordinateLayer, sizeof(coordinateLayer));

		fread(&coordinateLayer, sizeof(coordinateLayer), 1, file); // Right
		layerData.AnchorRight = util::StringAsInt(coordinateLayer, sizeof(coordinateLayer));

		tmpBytesRead += (sizeof(coordinateLayer) * 4);

		// Number of channels in the layer
		fread(&channelCount, sizeof(channelCount), 1, file);
		const auto nbrChannel = (unsigned short)util::StringAsInt(channelCount, sizeof(channelCount));
		tmpBytesRead += sizeof(channelCount);

		layerData.NbrChannel = nbrChannel;

		// Channel information. Six bytes per channel,
		unsigned char channelID[2];
		for (int i = 0; i < nbrChannel; i++)
		{
			fread(&channelID, sizeof(channelID), 1, file);
			short channelId = (short) util::StringAsInt(channelID, sizeof(channelID));
			layerData.ChannelId.push_back(channelId);
			fread(&dataLength, sizeof(dataLength), 1, file);
			int length = util::StringAsInt(dataLength, sizeof(dataLength));
			layerData.ChannelLength.push_back(length);
			tmpBytesRead += sizeof(channelID);
			tmpBytesRead += sizeof(dataLength);
		}

		// Blend mode signature: '8BIM'
		char signature[4];
		fread(&signature, sizeof(signature), 1, file);
		tmpBytesRead += sizeof(signature);
		if (!CheckSignatureLayerInfo(signature))
		{
			return false;
		}

		// Blend mode key
		fread(&dataLength, sizeof(dataLength), 1, file);
		tmpBytesRead += sizeof(dataLength);

		// Opacity + Clipping + Flag + Filler (zero) each 1 byte
		unsigned char blendValues[4];
		fread(&blendValues, sizeof(blendValues), 1, file);
		tmpBytesRead += sizeof(blendValues);

		bytesRead += tmpBytesRead;
		return true;
	}

	//----------------------------------------------------------------------------------------
	bool LayerAndMaskReader::ReadLayerInfoSectionExtraDataField(FILE* file, int& bytesRead, LayerData& layerData)
	{
		unsigned char dataLength[4];
		unsigned char data[1];
		int tmpBytesRead = 0;

		//	Length of the extra data field(= the total length of the next five fields).
		fread(&dataLength, sizeof(dataLength), 1, file);
		int extraFieldLength = util::StringAsInt(dataLength, sizeof(dataLength));
		tmpBytesRead += sizeof(dataLength);

		// Layer mask / adjustment layer data
		fread(&dataLength, sizeof(dataLength), 1, file);
		int layerMaskDataLength = util::StringAsInt(dataLength, sizeof(dataLength));
		tmpBytesRead += sizeof(dataLength);

		for (int i = 0; i < layerMaskDataLength; i++)
		{
			fread(&data, sizeof(data), 1, file);
			tmpBytesRead += sizeof(data);
		}

		// Layer blending ranges data
		fread(&dataLength, sizeof(dataLength), 1, file);
		int layerBlendingRangesDataLength = util::StringAsInt(dataLength, sizeof(dataLength));
		tmpBytesRead += sizeof(dataLength);

		for (int i = 0; i < layerBlendingRangesDataLength; i++)
		{
			fread(&data, sizeof(data), 1, file);
			tmpBytesRead += sizeof(data);
		}

		// size string
		unsigned char sizeString[1];
		fread(&sizeString, sizeof(sizeString), 1, file);
		int size = util::StringAsInt(sizeString, sizeof(sizeString));
		tmpBytesRead += sizeof(sizeString);

		// string name layer
		unsigned char* nameChars = new unsigned char[size];
		fread(nameChars, size * sizeof(unsigned char), 1, file);
		std::string name = util::PascalString(size, nameChars); // TODO: Handle unicode characters, 'luni' field and others
		tmpBytesRead += (size * sizeof(unsigned char));
		layerData.LayerName = name;

		// junk from pascal string padded to a multiple of 4 bytes
		const int junkValueSize = 3 - (size % 4);
		if (junkValueSize != 4)
		{
			for (int i = 0; i < junkValueSize; i++)
			{
				fread(nameChars, sizeof(data), 1, file);
				tmpBytesRead += sizeof(data);
			}
		}

		ReadHeaderAdditionalLayerInfo(file, tmpBytesRead, layerData);

		// Read the extra size 
		const int sizeToRead = extraFieldLength - tmpBytesRead + 4; // I don't like the +4 but for test.
		for (int i = 0; i < sizeToRead; i++)
		{
			fread(&data, sizeof(data), 1, file);
			tmpBytesRead += sizeof(data);
		}
		bytesRead += tmpBytesRead;
		return true;
	}

	//----------------------------------------------------------------------------------------
	bool LayerAndMaskReader::ReadChannelImageData(FILE* file, int& bytesRead, int channelDepth, LayerAndMaskData& layerMaskData, ProgressJob& progress)
	{
		unsigned char compression[2];

		progress.SetLabel("Reading image data..."); // TODO: localize string
		progress.Reset(layerMaskData.LayerCount());
		for( int layerIndex = 0; (layerIndex < layerMaskData.LayerCount()) && (!progress.IsCancelled()); layerIndex++ )
		{
			int const nbrChannels = layerMaskData.Layers[layerIndex].NbrChannel;
			int const length = (layerMaskData.Layers[layerIndex].AnchorBottom - layerMaskData.Layers[layerIndex].AnchorTop)
				* (layerMaskData.Layers[layerIndex].AnchorRight - layerMaskData.Layers[layerIndex].AnchorLeft);
			
			const int byteperchannel = channelDepth / 8;
			progress.NextTask();

			for( int channelIndex = 0; (channelIndex < nbrChannels) && (!progress.IsCancelled()); channelIndex++ )
			{
				// No pixel for user mask
				if(layerMaskData.Layers[layerIndex].ChannelId[channelIndex] < -1)
				{
					unsigned char data[1];
					int size = layerMaskData.Layers[layerIndex].ChannelLength[channelIndex];
					for (auto k = 0; k < size; k++)
					{
						fread(&data, sizeof(data), 1, file);
						bytesRead += sizeof(data);
					}
					continue;
				}

				// Compression value
				fread(&compression, sizeof(compression), 1, file);
				const short compressionValue = short(util::StringAsInt(compression, sizeof(compression)));
				bytesRead += sizeof(compression);

				// -------------- RAW ------------------
				switch (compressionValue)
				{
				case 0:
				{
					auto* uncompressedData = static_cast<unsigned char *>(_aligned_malloc(length,16));
					for (int k = 0; k < length; k++)
					{
						unsigned char value;
						fread(&value, sizeof(value), 1, file);
						uncompressedData[k] = value;
						bytesRead += sizeof(value);
					}
					layerMaskData.Layers[layerIndex].ImageContent.push_back(uncompressedData);
					break;
				}
				// ----------------- RLE ------------------
				case 1:
				{
					
					const int rowCount = (layerMaskData.Layers[layerIndex].AnchorBottom - layerMaskData.Layers[layerIndex].AnchorTop);
					short* sizes = new short[rowCount];

					unsigned char linePixelCount[2];
					int lengthGlobal = 0;
					int indexPosition = 0;
					auto* uncompressedData = static_cast<unsigned char *>(_aligned_malloc(length,16));

					// Read row size information
					for (int k = 0; k < rowCount; k++)
					{
						fread(linePixelCount, sizeof(linePixelCount), 1, file);
						sizes[k] = short(util::StringAsInt(linePixelCount, sizeof(linePixelCount)));
						lengthGlobal += sizes[k];
						bytesRead += sizeof(linePixelCount);
					}

					// Read data
					for (int k = 0; k < rowCount; k++)
					{
						for (int l = 0; l < sizes[k]; )
						{
							// TODO: bug, extreme lag and UI stutter apparently caused in this section

							unsigned char data[1];
							// PackBits
							fread(&data, sizeof(data), 1, file);
							bytesRead += sizeof(data);
							short size = short(util::StringAsInt(data, sizeof(data)));
							l++;
							if (size > 128)
							{
								size = 256 - size;
								fread(data, sizeof(data), 1, file);
								l++;
								bytesRead += sizeof(data);

								// optimized
								memset( uncompressedData + indexPosition, *data, (size+1) );
								indexPosition += (size+1);

								// unoptimized
								//for (short h = 0; h <= size; h++)
								//{
								//	uncompressedData[indexPosition] = *data;
								//	indexPosition++;
								//	progress.GetTask().SetValue( h/(float)size ); // SetValueAndUpdate( h/(float)size )
								//}
							}
							else if (size < 128)
							{
								for (short h = 0; h <= size; h++)
								{
									fread(data, sizeof(data), 1, file);
									bytesRead += sizeof(data);
									uncompressedData[indexPosition] = *data;
									indexPosition++;
									l++;
								}
							}
						}
					}
					layerMaskData.Layers[layerIndex].ImageContent.push_back(uncompressedData);
					break;
				}
				// -------  ZIP without prediction --------
				// ---------  ZIP with prediction ---------
				case 2:
				case 3:
				{
					int rows = (layerMaskData.Layers[layerIndex].AnchorBottom - layerMaskData.Layers[layerIndex].AnchorTop);
					int cols = (layerMaskData.Layers[layerIndex].AnchorRight - layerMaskData.Layers[layerIndex].AnchorLeft);
					int rowBytes = (cols * channelDepth + 7) / BYTE_VALUE;
					
					int lengthChannel = layerMaskData.Layers[layerIndex].ChannelLength[channelIndex];
					auto*zipdata = static_cast<unsigned char *>(_aligned_malloc(lengthChannel,16));
					const int count = int(fread(zipdata, 1, lengthChannel - 2, file)); // -2 is a value find in a another code of psd reader.

					auto* unzipdata = static_cast<unsigned char *>(_aligned_malloc(rows * rowBytes,16));
					bool unzipSuccess;
					if (compressionValue == 2)
					{
						unzipSuccess = psd_unzip_without_prediction(zipdata, count, unzipdata, rows * rowBytes);
					}
					else
					{
						unzipSuccess = psd_unzip_with_prediction(zipdata, count, unzipdata, rows * rowBytes, cols, channelDepth);
					}

					if(unzipSuccess)
					{
						layerMaskData.Layers[layerIndex].ImageContent.push_back(unzipdata);
					}
					
					break;
				}
				default: ;
				}
			}
		}
		return true;
	}

	//----------------------------------------------------------------------------------------
	bool LayerAndMaskReader::ReadGlobalLayerMaskInfo(FILE* file, int& bytesRead, LayerAndMaskData& layerMaskData)
	{
		unsigned char dataLength[4];

		fread(&dataLength, sizeof(dataLength), 1, file);
		const int lengthGlobal = util::StringAsInt(dataLength, sizeof(dataLength));
		bytesRead += sizeof(dataLength);

		if (lengthGlobal == 0) return 0;

		int tmpBytesRead = 0;
		unsigned char data[1];
		while (tmpBytesRead < lengthGlobal)
		{
			fread(&data, sizeof(data), 1, file);
			tmpBytesRead += sizeof(data);
		}

		bytesRead += tmpBytesRead;
		return true;
	}

	//----------------------------------------------------------------------------------------
	bool LayerAndMaskReader::ReadHeaderAdditionalLayerInfo(FILE* file, int& bytesRead, LayerData& layerData)
	{
		int tmpBytesRead = 0;
		char signature[4];

		// SIGANTURE
		fread(&signature, sizeof(signature), 1, file);
		tmpBytesRead += sizeof(signature);

		while (CheckSignatureLayerInfo(signature))
		{
			AdditionnalLayerDataLayer(file, bytesRead, layerData);

			// Try read SIGANTURE
			fread(&signature, sizeof(signature), 1, file);
			tmpBytesRead += sizeof(signature);
		}

		// Check signature failed so go back of 4 bytes, size of the signature.
		fseek(file, -4, SEEK_CUR);
		bytesRead += tmpBytesRead;
		return true;
	}


	//----------------------------------------------------------------------------------------
	bool LayerAndMaskReader::ReadHeaderAdditionalLayerGlobalInfo(FILE* file, int& bytesRead, const HeaderData& headerData, LayerAndMaskData& layerDataMaskData, ProgressJob& progress)
	{
		int tmpBytesRead = 0;
		char signature[4];

		// SIGANTURE
		fread(&signature, sizeof(signature), 1, file);
		tmpBytesRead += sizeof(signature);

		while (CheckSignatureLayerInfo(signature))
		{
			AdditionnalLayerDataGlobal(file, bytesRead, headerData, layerDataMaskData, progress);

			// Try read SIGANTURE
			fread(&signature, sizeof(signature), 1, file);
			tmpBytesRead += sizeof(signature);
		}

		// Check signature failed so go back of 4 bytes, size of the signature.
		fseek(file, -4, SEEK_CUR);
		bytesRead += tmpBytesRead;
		return true;
	}

	//----------------------------------------------------------------------------------------
	void LayerAndMaskReader::AdditionnalLayerDataGlobal(FILE* file, int& bytesRead, const HeaderData& headerData, LayerAndMaskData& layerDataMaskData, ProgressJob& progress)
	{
		char dataKey[4];
		unsigned char dataLength[4];

		// KEY
		fread(&dataKey, sizeof(dataKey), 1, file);
		bytesRead += sizeof(dataKey);
		const std::string key(dataKey, 4);

		// LAYERS
		if (key == KEY_LAYERS)
		{
			ProcessLayerMaskInformation(file, bytesRead, headerData, layerDataMaskData, progress);
			return;
		}

		// LENGHT
		fread(&dataLength, sizeof(dataLength), 1, file);
		unsigned int size = (unsigned int)util::StringAsInt(dataLength, sizeof(dataLength));
		bytesRead += sizeof(dataLength);
		size += (size % 2);
		bytesRead += size * sizeof(dataLength[0]);

		fseek(file, size, SEEK_CUR);
	}

	//----------------------------------------------------------------------------------------
	void LayerAndMaskReader::AdditionnalLayerDataLayer(FILE* file, int& bytesRead, LayerData& layerData)
	{
		char dataKey[4];
		unsigned char dataLength[4];

		// KEY
		fread(&dataKey, sizeof(dataKey), 1, file);
		bytesRead += sizeof(dataKey);
		std::string key(dataKey, 4);

		// LENGHT
		fread(&dataLength, sizeof(dataLength), 1, file);
		unsigned int size = (unsigned int)util::StringAsInt(dataLength, sizeof(dataLength));
		bytesRead += sizeof(dataLength);
		size += (size % 2);
		bytesRead += size;

		// Key group
		if (key == KEY_GROUP)
		{
			SectionDividerSetting(file, size, layerData);
			return;
		}

		// Key group
		if (key == KEY_VECTOR_MASK || key == KEY_MASK)
		{
			ReadVectorMask(file, size, layerData);
			return;
		}

		fseek(file, size, SEEK_CUR);
	}

#pragma region ADDITIONNAL LAYER DATA

	//----------------------------------------------------------------------------------------
	void LayerAndMaskReader::SectionDividerSetting(FILE* file, int const& size, LayerData& layerData)
	{
		unsigned char typeData[4];

		// type layer
		fread(&typeData, sizeof(typeData), 1, file);
		const unsigned int type = util::StringAsInt(typeData, sizeof(typeData));
		layerData.Type = static_cast<TYPE_LAYER>(type);

		if (size <= 4) return;

		fseek(file, size, SEEK_CUR);
	}


	//----------------------------------------------------------------------------------------
	void LayerAndMaskReader::ReadVectorMask(FILE* file, int const& size, LayerData& layerData)
	{
		unsigned char typeData[4];

		// Type version
		fread(&typeData, sizeof(typeData), 1, file);
		// Flag
		fread(&typeData, sizeof(typeData), 1, file);
		
		const int sizeRecordPath = (size - 8);
		ReadPaths(file, layerData.PathRecords, sizeRecordPath);

		const int sizeJunk = sizeRecordPath % PATH_BLOCK_SIZE;
		
		fseek(file, sizeJunk, SEEK_CUR);
	}

	//----------------------------------------------------------------------------------------
	// TODO: Duplicate code from ImageResourceReader::ReadPaths()
	bool LayerAndMaskReader::ReadPaths(FILE* file, std::vector<util::PathRecord> & pathRecords, int sizeBlock)
	{
		if (sizeBlock <= 0) return false;
		unsigned char selectorData[2];
		const int size = sizeBlock / 26;

		PathRecord pathRecord;
		for (auto i = 0; i < size; i++)
		{
			fread(selectorData, sizeof(selectorData), 1, file);
			const auto selector = short(util::StringAsInt(selectorData, sizeof(selectorData)));
			switch (selector)
			{
			case 0:
			case 3:
			{
				if (!pathRecord.Points.empty() && !(pathRecord.IsClosedPath && pathRecord.Points.size() <= 2))
				{
					pathRecords.push_back(pathRecord);
				}

				pathRecord =  PathRecord();
				pathRecord.IsClosedPath = (selector == 0);

				unsigned char pathRecordData[24];
				fread(pathRecordData, sizeof(pathRecordData), 1, file);
				break;
			}
			case 1:
			case 4:
			case 2:
			case 5:
			{
				PathPoints* points = ReadPathPoint(file);
				points->IsLinked = (selector == 1 || selector == 4);
				pathRecord.Points.push_back(points);
				break;
			}
			default:
			{
				unsigned char pathRecordData[24];
				fread(pathRecordData, sizeof(pathRecordData), 1, file);
				break;
			}
			}
		}

		if (!pathRecord.Points.empty() && !(pathRecord.IsClosedPath && pathRecord.Points.size() <= 2))
		{
			pathRecords.push_back(pathRecord);
		}
		return true;
	}

	//----------------------------------------------------------------------------------------
	PathPoints* LayerAndMaskReader::ReadPathPoint(FILE * file)
	{

		unsigned char pathRecordData[24];
		PathPoints* pathPoints = new PathPoints();

		fread(pathRecordData, sizeof(pathRecordData), 1, file);

		pathPoints->SegIn.y = ImageResourceReader::fixedPoint(&pathRecordData[0], true, 32, 8);
		pathPoints->SegIn.x = ImageResourceReader::fixedPoint(&pathRecordData[4], true, 32, 8);

		pathPoints->AnchorPoint.y = ImageResourceReader::fixedPoint(&pathRecordData[8], true, 32, 8);
		pathPoints->AnchorPoint.x = ImageResourceReader::fixedPoint(&pathRecordData[12], true, 32, 8);

		pathPoints->SegOut.y = ImageResourceReader::fixedPoint(&pathRecordData[16], true, 32, 8);
		pathPoints->SegOut.x = ImageResourceReader::fixedPoint(&pathRecordData[20], true, 32, 8);

		return pathPoints;
	}

	//----------------------------------------------------------------------------------------
	bool LayerAndMaskReader::psd_unzip_without_prediction(unsigned char *src_buf, int src_len, unsigned char *dst_buf, int dst_len)
	{
		z_stream stream;
		int state;

		memset(&stream, 0, sizeof(z_stream));
		stream.data_type = Z_BINARY;

		stream.next_in = (Bytef *)src_buf;
		stream.avail_in = src_len;
		stream.next_out = (Bytef *)dst_buf;
		stream.avail_out = dst_len;

		if (inflateInit(&stream) != Z_OK) return true;

		do {
			state = inflate(&stream, Z_PARTIAL_FLUSH);
			if (state == Z_STREAM_END) break;
			if (state == Z_DATA_ERROR || state != Z_OK) break;

		} while (stream.avail_out > 0);

		return state == Z_STREAM_END || state == Z_OK;
	}

	//----------------------------------------------------------------------------------------
	bool LayerAndMaskReader::psd_unzip_with_prediction(unsigned char *src_buf, int src_len,	unsigned char *dst_buf, int dst_len, int row_size, int color_depth)
	{
		const bool res = psd_unzip_without_prediction(src_buf, src_len, dst_buf, dst_len);
		if (!res) return res;

		unsigned char *buf = dst_buf;
		do {
			int len = row_size;
			if (color_depth == 16)
			{
				while (--len)
				{
					buf[2] += buf[0] + ((buf[1] + buf[3]) >> 8);
					buf[3] += buf[1];
					buf += 2;
				}
				buf += 2;
				dst_len -= row_size * 2;
			}
			else
			{
				while (--len)
				{
					*(buf + 1) += *buf;
					buf++;
				}
				buf++;
				dst_len -= row_size;
			}
		} while (dst_len > 0);
		return res;
	}

#pragma endregion
}