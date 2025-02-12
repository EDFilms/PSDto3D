//----------------------------------------------------------------------------------------------
// ===============================================
//  Copyright (C) 2016, CDRIN.
//  All Rights Reserved.
// ===============================================
//  Unauthorized copying of this file, via any medium is strictly prohibited
//  Proprietary and confidential
//
//  @file textureExporter.cpp
//  @author Fabien Raspail
//  @date 11-11-2016
//
//  @section DESCRIPTION
//
//----------------------------------------------------------------------------------------------

// TODO: Need this, otherwise error linking method QByteArray QString::toUtf8() const & Q_REQUIRED_RESULT
//#define QT_COMPILING_QSTRING_COMPAT_CPP // added in project settings
// Libpng config, required before header include
#define PNGLCONF_H
#define PNG_LINKAGE_API
#define PNG_STDIO_SUPPORTED
#define PNG_INFO_IMAGE_SUPPORTED
#define PNG_WRITE_SUPPORTED

#include "png.h"
#include "textureExporter.h"
#include "psd_reader/layerAndMaskReader.h"
#include "util/utils.h"
#include "util/helpers.h"
#if defined PSDTO3D_MAYA_VERSION
#include <maya/MTextureManager.h>
#else
#include "mayaStub.h"
#endif
#include <algorithm>
#include <chrono> // for std::chrono::high_resolution_clock

// Libpng helpers
const char* libpng_user_error_ptr = "ERROR";
void libpng_user_error_fn(png_structp png_ptr,png_const_charp error_msg)		{ }
void libpng_user_warning_fn(png_structp png_ptr,png_const_charp warning_msg)	{ }

namespace util { void DebugPrint(char* lpszFormat, ...); }; // TODO: Delete this, testing only

namespace psd_to_3d
{
	//--------------------------------------------------------------------------------------------------------------------------------------
	template<class T> void TexturePixel<T>::Defringe( const TexturePixel<T>& src )
	{
		bool thisTransparent = (this->a==0), srcTransparent = (src.a==0);
		if( thisTransparent && !srcTransparent )
			this->r = src.r, this->g = src.g, this->b = src.b;
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void TextureMap::Init( int width, int height ) // rgba 4-channel
	{
		this->width = width;
		this->height = height;
		int pixel_size = 4; // assume output is rgba 1 byte per channel
		p.resize( width * height * pixel_size);
	}

// ApplyScaling() fails due to optimizer bug in VS2022
#pragma optimize("",off) // TODO: rewrite ApplyScaling() and remove this
	//--------------------------------------------------------------------------------------------------------------------------------------
	// Change bitmap size and resample; only supports downscaling, not upscaling
	void TextureMap::ApplyScaling( int width_output, int height_output )
	{
		if( (width_output >= this->width) || (height_output >= this->height) ||		// same size or upscaling
			(width_output < 1) || (height_output < 1) )								// invalid size
		{
			return;
		}

		int width_input  = this->width;
		int height_input = this->height;
		int pixel_size = 4; // assume result and original are both rgba 1 byte per channel

		std::vector<unsigned char> output; // result texture
		output.resize( width_output * height_output * pixel_size );

		// number of input pixels horizontally or vertically associated with a single output pixel
		// assumes square aspect ratio
		double pixel_span = width_input / (double)width_output;
		double pixel_area = pixel_span * pixel_span;
		int output_index = 0;

		for( int y=0; y<height_output; y++ )
		{
			double yf_lo  = y * pixel_span;
			double yf_hi = (y+1) * pixel_span;
			int yi_lo = (int)yf_lo;
			int yi_hi = MIN( (int)yf_hi, (height_input-1) );
			double ymix_lo = (1.0 - (yf_lo - yi_lo));
			double ymix_hi = (yf_hi - yi_hi);

			for( int x=0; x<width_output; x++, output_index+=4 )
			{
				double xf_lo  = x * pixel_span;
				double xf_hi = (x+1) * pixel_span;
				int xi_lo = (int)xf_lo;
				int xi_hi = MIN( (int)xf_hi, (width_input-1) );
				double xmix_lo = (1.0 - (xf_lo - xi_lo));
				double xmix_hi = (xf_hi - xi_hi);

				double r=0, g=0, b=0, a=0;
				for( int i=yi_lo; i<=yi_hi; i++)
				{
					for( int j=xi_lo; j<=xi_hi; j++ )
					{
						double mul = (i==yi_lo? ymix_lo : (i==yi_hi? ymix_hi : 1.0));
						mul *= (j==xi_lo? xmix_lo : (j==xi_hi? xmix_hi : 1.0));

						// grab the input pixel
						int index_input = ((i*width_input) + j) * pixel_size;

						unsigned int rgba = *(unsigned int*)(p.data()+index_input);
						r += ((rgba & 0x000000FF) >> 0) * mul;
						g += ((rgba & 0x0000FF00) >> 8) * mul;
						b += ((rgba & 0x00FF0000) >> 16) * mul;
						a += ((rgba & 0xFF000000) >> 24) * mul;
					}
				}
				unsigned int rgba = 
					(((unsigned int)round(r / pixel_area)) << 0) +
					(((unsigned int)round(g / pixel_area)) << 8) +
					(((unsigned int)round(b / pixel_area)) << 16) +
					(((unsigned int)round(a / pixel_area)) << 24);
				*(unsigned int*)(output.data() + output_index ) = rgba;
			}
		}

		this->p.swap( output );
		this->width = width_output;
		this->height = height_output;
	}
#pragma optimize("",on) // see note above

	//--------------------------------------------------------------------------------------------------------------------------------------
	void TextureMap::ApplyDefringe( int radius )
	{
		const int pixel_size = 4; // assume rgba 1 byte per channel
		png_byte* row_base = (png_byte*)(p.data());

		for( int y_cur=0; y_cur<height; y_cur++ )
		{
			png_byte* row = row_base + (width*y_cur*pixel_size);
			for( int x_cur=0; x_cur<width; x_cur++ )
			{
				TexturePixel<unsigned char>* pix_cur = (TexturePixel<unsigned char>*)(row+(x_cur*pixel_size)); // current pixel
				unsigned char alpha_cur = pix_cur->a;
				if( alpha_cur == 0 )
				{
					bool done_pixel = false;
					int y_lo = MAX(0, y_cur-radius), y_hi = MIN(height, y_cur+radius);
					int x_lo = MAX(0, x_cur-radius), x_hi = MIN(width, x_cur+radius);
					for( int y_cmp = y_lo;  (y_cmp < y_hi) && !done_pixel;  y_cmp++ )
					{
						png_byte* row_cmp = row_base + (width*y_cmp*pixel_size);
						for( int x_cmp = x_lo;  (x_cmp < x_hi) && !done_pixel;  x_cmp++ )
						{
							TexturePixel<unsigned char>* pix_cmp = (TexturePixel<unsigned char>*)(row_cmp+(x_cmp*pixel_size)); // comparison pixel
							unsigned char alpha_cmp = pix_cmp->a;
							if( pix_cmp->a > 0 )
							{
								pix_cur->r = pix_cmp->r, pix_cur->g = pix_cmp->g, pix_cur->b = pix_cmp->b;
								done_pixel = true;
								//pix_cur->Defringe( *pix_cmp );
							}
						}
					}
				}
			}
		}
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void TextureMap::ApplyPadding( int padding )
	{
		if( padding==0 )
			return;

		int width_input  = this->width;
		int height_input = this->height;
		int width_output  = this->width  + (2*padding);
		int height_output = this->height + (2*padding);
		int pixel_size = 4; // assume result and original are both rgba 1 byte per channel

		std::vector<unsigned char> output; // result texture
		output.resize( width_output * height_output * pixel_size );

		int line_size_input = width_input*pixel_size; // bytes per line of image before padding
		int line_size_output = width_output*pixel_size; // bytes per line of image after padding
		// apply top and bottom rows
		for( int y=0; y < padding; y++ )
		{
			// top row
			int offset_output = (y*line_size_output);
			memset( output.data() + offset_output, 0, line_size_output );
			// bottom row
			offset_output = ((y+height_input+padding)*line_size_output);
			memset( output.data() + offset_output, 0, line_size_output );
		}

		for (int y=0; y < height_input; y++)
		{
			// left column
			int offset_input = 0;
			int offset_output = ((y+padding)*line_size_output);
			memset( output.data() + offset_output, 0, padding*pixel_size );

			// center contents
			offset_input = (y*line_size_input);
			offset_output = offset_output + (padding*pixel_size);
			memcpy( output.data() + offset_output, p.data() + offset_input, line_size_input );

			// right column
			offset_output = offset_output + line_size_input;
			memset( output.data() + offset_output, 0, padding*pixel_size );
		}
		this->p.swap( output );
		this->width = width_output;
		this->height = height_output;
	}

	#include <chrono>
	typedef std::chrono::time_point<std::chrono::high_resolution_clock> time_point;
	double perf_log_total = 0.0; // TODO: Create full PerfLog class, or whatever it's 
	int perf_log_count = 0;
	unsigned long long total_pixels = 0;
	unsigned int total_files = 0;

	//--------------------------------------------------------------------------------------------------------------------------------------
	template <int byte_depth_input, int has_alpha>
	inline void TConvertIffFormat( TextureMap& tex, const psd_reader::LayerData& layer, const boundsPixels rect )
	{
		time_point time_start = std::chrono::high_resolution_clock::now();
		perf_log_count++;

		 // only support rgb or rgba
		if( (layer.NbrChannel<3) ) // || (layer.NbrChannel>4) // some layers have 5 or more channels, maybe for the mask
			return ;

		int width  = rect.WidthPixels();
		int height = rect.HeightPixels();
		int pixel_size_output = 4; // assume rgba 1 byte per channel
		int index = 0;

		// the input layer only contains pixel data within the anchor region
		int rowSize = layer.AnchorRight - layer.AnchorLeft;

		// fill content
		unsigned char* data = tex.Data();
		int i_start = rect.TopPixel(),  i_end = rect.BottomPixel();
		int j_start = rect.LeftPixel(), j_end = rect.RightPixel();
		int padLeft = std::max(0, j_start-layer.AnchorLeft);

		// TODO: Check the _start and _end ranges are correct;  Test using image with 1-pixel red outer border

		// helper variables
		const int imageContentIndices_rgba[4] = {1,2,3,0};
		const int imageContentIndices_rgb[4] = {0,1,2,-1};
		const int* ndx = (has_alpha? imageContentIndices_rgba:imageContentIndices_rgb);

		for (int i = i_start; i < i_end; i++) // input pixel position, vertical Y-coord
		{
			int pos = (((i - layer.AnchorTop) * rowSize) + padLeft) * byte_depth_input; // index of the first needed pixel of this row in the input layer
			float accumR = 0, accumG = 0, accumB = 0, accumA = 0, accumSpan = 0; // RGBA and total weight accumulation values

			bool isOutsideAnchorY = (i < layer.AnchorTop || i >= layer.AnchorBottom);

			int j_content_start = MAX( layer.AnchorLeft, rect.LeftPixel() );
			int j_content_end = MIN( layer.AnchorRight, rect.RightPixel() );

			// using separate loops for "outside" and "content" regions improves performance

			// CASE 1:  top and bottom blank pixels outside the contents
			if( isOutsideAnchorY )
			{
				for (int j = j_start; j < j_end; j++) // input pixel position, horizontal X-coord
				{
					memset( data+index, 0, pixel_size_output ); // blank, zero alpha pixel
					index += pixel_size_output;
				}
			}
			else
			{
				// CASE 2:  left blank pixels before the contents
				for (int j = j_start; j < j_content_start; j++) // image pixel position, horizontal
				{
					memset( data+index, 0, pixel_size_output ); // blank, zero alpha pixel
					index += pixel_size_output;

					bool isOutsideAnchorX = (j < layer.AnchorLeft);
					if( !isOutsideAnchorX ) // if position is within the image, need to advance image pointer
						pos += byte_depth_input;
				}

				// CASE 3:  main body, center content pixels
				for (int j = j_content_start; j < j_content_end; j++) // image pixel position, horizontal
				{
					// TODO: for performance, this should read and write 64-bit aligned values
					// each iteration, instead of one byte at a time

					// read starting at 'pos' in the input
					unsigned int r=0, g=0, b=0, a=0xFFFFFFFF;
					memcpy( &r, layer.ImageContent[ndx[0]]+pos, byte_depth_input);
					memcpy( &g, layer.ImageContent[ndx[1]]+pos, byte_depth_input);
					memcpy( &b, layer.ImageContent[ndx[2]]+pos, byte_depth_input);
					if( has_alpha )
						memcpy( &a, layer.ImageContent[ndx[3]]+pos, byte_depth_input);

					*(data+index+0) = (unsigned char)(r>>(8*(byte_depth_input-1)));
					*(data+index+1) = (unsigned char)(g>>(8*(byte_depth_input-1)));
					*(data+index+2) = (unsigned char)(b>>(8*(byte_depth_input-1)));
					*(data+index+3) = (unsigned char)(a>>(8*(byte_depth_input-1)));

					index += pixel_size_output; // advance position of output, assume output is rgba 1 byte per channel
					pos += byte_depth_input; // advance position of input, applies to each color channel
				}

				// CASE 4:  right blank pixels after the contents
				for (int j = j_content_end; j < j_end; j++) // image pixel position, horizontal
				{
					memset( data+index, 0, pixel_size_output ); // blank, zero alpha pixel
					index += pixel_size_output;

					bool isOutsideAnchorX = (j >= layer.AnchorRight);
					if( !isOutsideAnchorX ) // if position is within the image, need to advance image pointer
						pos += byte_depth_input;
				}
			}
		}

		time_point time_now = std::chrono::high_resolution_clock::now();
		std::chrono::duration<float> time_diff = time_now-time_start;
		perf_log_total += time_diff.count();

	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void TextureExporter::ConvertIffFormat(TextureMap& tex, const psd_reader::LayerData& layer,
		int srcBitDepth, const boundsPixels region )
	{
		bool has_alpha = (layer.NbrChannel>3);
		const int byte_depth_input = srcBitDepth / 8;

		// template usage slightly improves performance
		if( has_alpha )
		{
			if( byte_depth_input==1 ) return TConvertIffFormat<1,1>( tex, layer, region );
			if( byte_depth_input==2 ) return TConvertIffFormat<2,1>( tex, layer, region );
			if( byte_depth_input==3 ) return TConvertIffFormat<3,1>( tex, layer, region );
			if( byte_depth_input==4 ) return TConvertIffFormat<4,1>( tex, layer, region );
		}
		else
		{
			if( byte_depth_input==1 ) return TConvertIffFormat<1,0>( tex, layer, region );
			if( byte_depth_input==2 ) return TConvertIffFormat<2,0>( tex, layer, region );
			if( byte_depth_input==3 ) return TConvertIffFormat<3,0>( tex, layer, region );
			if( byte_depth_input==4 ) return TConvertIffFormat<4,0>( tex, layer, region );
		}
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void TextureExporter::ConvertToMask(TextureMap& tex, const psd_reader::LayerData& layer, int width, int height, int depth)
	{
		const int bytesPerPixel = depth / 8;
		//maskTexture.reserve(width * height * bytesPerPixel);

		int padTop = std::max(0, (0 - layer.AnchorTop));
		int padHeightTop = std::max(0, layer.AnchorTop);
		int padLeft = std::max(0, (0 - layer.AnchorLeft));
		int padRight = std::max(0, (layer.AnchorRight - width));

		int rowSize = layer.AnchorRight - layer.AnchorLeft;
		int globalPaddingTop = padTop * rowSize;

		// fill content
		int indexProgression = 0;
		for (int i = 0; i < height; i++)
		{
			for (int j = 0; j < width; j++)
			{
				if( (i < layer.AnchorTop || i >= layer.AnchorBottom || j < layer.AnchorLeft || j >= layer.AnchorRight) )
				{
					for( int k = 0; k < bytesPerPixel; k++ )
					{
						tex.Buffer().push_back(0);
					}
				}
				else
				{
					int pos = indexProgression + ((globalPaddingTop + (padLeft * (i - padHeightTop + 1)) + (padRight * (i - padHeightTop))) * bytesPerPixel);

					for( int k = 0; k < bytesPerPixel; k++ )
					{
						tex.Buffer().push_back(layer.ImageContent[1][pos + k]);
					}
					indexProgression += bytesPerPixel;
				}
			}
		}
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	boundsPixels TextureExporter::GetContentRegion(const psd_reader::LayerData& layer,int width, int height, int depth, int alphaThresh)
	{
		const int bytesPerPixel = depth / 8;
		if (layer.NbrChannel < 4) // no alpha, assume layer covers entire comp
			return boundsPixels(0,0,width,height);
		int pos = 0;
		int xLo = width, xHi = 0, yLo = height, yHi = 0;

		for (int i = layer.AnchorTop; i < layer.AnchorBottom; i++) // image pixel position, vertical
		{
			for (int j = layer.AnchorLeft; j < layer.AnchorRight; j++) // image pixel position, horizontal
			{
				// TODO: assumes alpha is 8-bits, fix to allow 16-bit
				unsigned char alpha = layer.ImageContent[0][pos];
				if( alpha>alphaThresh )
				{
					if( j<xLo ) xLo = j;
					if( j>xHi ) xHi = j;
					if( i<yLo ) yLo = i;
					if( i>yHi ) yHi = i;
				}
				pos += bytesPerPixel;
			}
		}
		return boundsPixels( xLo, yLo, 1+(xHi-xLo), 1+(yHi-yLo) );
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	// Defringe spreads colors from non-transparent pixels into nearby transparent pixels,
	// to reduce fringing artifacts which can occur when contrasting colors exist in transparent pixels of a texture
	void TextureExporter::Defringe(TextureMap& tex, int radius)
	{
		tex.ApplyDefringe(radius);
	}


	//--------------------------------------------------------------------------------------------------------------------------------------
	// Helper for SaveToDiskLibpng
	int ProgressTaskCallback( void* progress_param, float progress_val )
	{
		ProgressTask& progressTask = *((ProgressTask*)progress_param);
		progressTask.SetValueAndUpdate( progress_val );
		return (progressTask.IsCancelled()? 1:0);
	}


	//--------------------------------------------------------------------------------------------------------------------------------------
	void TextureExporter::SaveToDiskLibpng(const std::string filepath, std::vector<unsigned char>& srcTexture_, int width, int height, ProgressTask& progressTask)
	{
		if( progressTask.IsCancelled() ) return; // cancel handling

		// TODO: Delete this, testing only
		std::vector<unsigned char> srcTexture;
		int srcTexture_size = width * height * 4;
		srcTexture.resize( srcTexture_size + 1024 ); // 
		memcpy( srcTexture.data(), srcTexture_.data(), srcTexture_size );

		total_files++;
		total_pixels += (width*height);

		png_structp png_ptr;
		png_infop info_ptr;

		// Open the file
		FILE *fp = NULL;
		errno_t err = _wfopen_s(&fp, util::to_utf16(filepath).c_str(), L"wb");
		if ((err != 0) || (fp == NULL))
			return; // ERROR

		// Create and initialize the png_struct with error handlers
		png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,
			(png_voidp)libpng_user_error_ptr, libpng_user_error_fn, libpng_user_warning_fn);
		if (png_ptr == NULL)
		{
			fclose(fp);
			return; // ERROR
		}

		// Allocate/initialize the image information data.
		info_ptr = png_create_info_struct(png_ptr);
		if (info_ptr == NULL)
		{
			fclose(fp);
			png_destroy_write_struct(&png_ptr,  NULL);
			return; // ERROR
		}

		// Configure the image information data.
		const int BYTES_PER_PIXEL = 4;
		const int BITS_PER_COMPONENT = 8; // bit depth
		const int COLOR_TYPE = PNG_COLOR_TYPE_RGB_ALPHA;
		const int INTERLACE_TYPE = PNG_INTERLACE_NONE;
		const int COMPRESSION_TYPE = PNG_COMPRESSION_TYPE_DEFAULT;
		const int FILTER_METHOD = PNG_FILTER_TYPE_DEFAULT;
		png_set_IHDR(png_ptr, info_ptr,
			width, height, BITS_PER_COMPONENT,
			COLOR_TYPE, INTERLACE_TYPE, COMPRESSION_TYPE, FILTER_METHOD );

		// Not using additional error handling, supplied handlers in png_create_write_struct()
		//if (setjmp(png_jmpbuf(png_ptr)))
		//{
		//   fclose(fp);
		//   png_destroy_write_struct(&png_ptr, &info_ptr);
		//   return; // ERROR
		//}

		// Set up the output control if you are using standard C streams.
		png_init_io(png_ptr, fp);

		// Allocate and configure row pointers
		png_byte** rows = new png_byte*[height];
		png_byte* row = (png_byte*)srcTexture.data();
		for( int y=0; y<height; y++ )
		{
			rows[y] = row;
			row += (BYTES_PER_PIXEL*width);
		}

		// Assign row pointers
		png_set_rows(png_ptr, info_ptr, rows);

		// Write PNG file.  All the image info ready in the structure.
		int png_transforms = PNG_TRANSFORM_IDENTITY;
		png_write_png(png_ptr, info_ptr, png_transforms, NULL, &ProgressTaskCallback, &progressTask);

		// Not using png_malloced palette, or trans array.
		// Free those here, when sure that libpng is through with them.
		//png_free(png_ptr, palette);
		//palette = NULL; // set to NULL to protect if calling png_free() again.
		//png_free(png_ptr, trans);
		//trans = NULL; // set to NULL to protect if calling png_free() again.

		// Clean up after the write, and free any allocated memory.
		png_destroy_write_struct(&png_ptr, &info_ptr);

		// Close the file.
		fclose(fp);

		delete[] rows;

		// SUCCESS
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void TextureExporter::SaveToDiskMTexture(const std::string filepath, std::vector<unsigned char>& srcTexture, int width, int height)
	{
		MHWRender::MRenderer* renderer = MHWRender::MRenderer::theRenderer();
		MHWRender::MTextureManager* textureMgr = renderer ? renderer->getTextureManager() : NULL;

		if (textureMgr)
		{
			unsigned char* fTextureData = srcTexture.data();
			MHWRender::MTextureDescription desc;
			{
				desc.setToDefault2DTexture();
				desc.fWidth = width;
				desc.fHeight = height;
				desc.fDepth = 1;
				desc.fBytesPerRow = width * 4;
				desc.fFormat = MHWRender::kR8G8B8A8_UNORM;
				desc.fTextureType = MHWRender::kImage2D;
			}
			MHWRender::MTexture* ftexture = textureMgr->acquireTexture("tmp_Export_texture_PSD2M", desc, fTextureData);
			textureMgr->saveTexture(ftexture, filepath.c_str());
			textureMgr->releaseTexture(ftexture);
		}
	}
}
