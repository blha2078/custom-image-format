/*
* Copyright 2021 Blake Hamilton. All rights reserved
* All tags in the source code that seem different are meant
* for automatic documentation generator. Automatic documentation
* generator used is DOXYGEN.
*
*/
/**
* \file *.cpp
* \brief *.* is a ...
* Example (optional):
* \code
* // C++ source code of a minimal example on how to use the API
* \endcode
* \author Blake Hamilton
*
* $Header: $
* $Log: $
*/

/*
* BLAKE IMAGE FORMAT (.bif)
*
* File Header:
* 4 BYTES -	Unique four letter character code to identify file type on read = BIF1
* 2 BYTES - Pixel Width
* 2 BYTES - Pixel Height
* 4 BYTES - Fill Color
*
* File Body:
* N BYTES - Pixel data - byte size is computed with formula ([Pixel Width] * [Pixel Height] * [Bytes Per Color Channel] * [Number Of Color Channels])
*
*/

// includes
#include "stdafx.h"
#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <stdio.h> 
#include <stdlib.h>
#include <Shlobj.h>
#include <time.h>
#include "resource.h"

// libs
#pragma comment(lib, "Shell32.lib")

// consts
const unsigned short FileVersion = 100;
const BYTE BifFourCC[4] = { 0x42, 0x49, 0x46, 0x46 }; // BIFF

// globals
BITMAP mBitmapObject = {};
HDC mMemoryHdc = NULL;

// forward declared functions

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Method		CreateImage
//	Purpose:	Creates a new BIF image file
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CreateImage(const char* filePath, unsigned short pixelWidth, unsigned short pixelHeight, COLORREF fillColor);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Method		DisplayImage
//	Purpose:	Reads a BIF image file and displays it in a window
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL DisplayImage(const char* filePath);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Method		ConfigureScreen
//	Purpose:	Prints usage to the screen
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ConfigureScreen();

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Method		CenterWindow
//	Purpose:	Centers a window
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CenterWindow(HWND hwndWindow);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Method		PrintUsageError
//	Purpose:	Prints a command line usage error
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PrintUsageError();

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Method		FileExists
//	Purpose:	Returns TRUE is file path exists
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL FileExists(const char* filePath);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Method		DirectoryExists
//	Purpose:	Returns TRUE is directory path exists
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL DirectoryExists(const char* directoryPath);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Method		PrintOsErrorText
//	Purpose:	Prints the friendly text associated with the last OS error message numeric code
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PrintOsErrorText();

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Method		WindowProc
//	Purpose:	Windos message loop function required for window
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// function impementations

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Method		main
//	Purpose:	application entry point
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// int main()
int main(int argc, char* argv[])
{
	// configure screen
	ConfigureScreen();

	// check arguments
	if (__argc < 7)
	{
		// print usage error
		PrintUsageError();
		
		// return failed status code
		return -1;
	}

	// print log information message
	printf("Validating command line arguments...\n");

	// pixel width parameter
	unsigned short pixelWidth = (unsigned short) atoi((const char*)__argv[1]);
	if (pixelWidth == 0)
	{
		// print usage error
		PrintUsageError();

		// return failed status code
		return -1;
	}

	// pixel height parameter
	unsigned short pixelHeight = (unsigned short) atoi((const char*)__argv[2]);
	if (pixelHeight == 0)
	{
		// print usage error
		PrintUsageError();

		// return failed status code
		return -1;
	}

	// red channel parameter
	BYTE red = (BYTE) atoi((const char*)__argv[3]);

	// green channel parameter
	BYTE green = (BYTE)atoi((const char*)__argv[4]);

	// blue channel parameter
	BYTE blue = (BYTE)atoi((const char*)__argv[5]);

	// file path parameter
	const char* filePath = (const char*) __argv[6];
	if (filePath == NULL)
	{
		// print usage error
		PrintUsageError();

		// return failed status code
		return -1;
	}

	// print log information message
	printf("Checking if %s already exists...\n", filePath);

	// file path exists
	if (FileExists(filePath) == TRUE)
	{
		// print log information message
		printf("Deleting file %s...\n", filePath);

		// delete existing file
		if (::DeleteFile(filePath) == FALSE)
		{
			// print message
			PrintOsErrorText();

			// return failed status code
			return -1;
		}
	}

	// split file path - fyi does not handle UNC paths
	char driveBuffer[_MAX_DRIVE] = "";
	char directoryBuffer[_MAX_DIR] = "";
	char fileNameBuffer[_MAX_FNAME] = "";
	char extensionBuffer[_MAX_EXT] = "";
	::_splitpath(filePath, driveBuffer, directoryBuffer, fileNameBuffer, extensionBuffer);

	// create directory path char array on the heap
	char* directoryPath = (char*) ::malloc(strlen(driveBuffer) + strlen(directoryBuffer) + 1);
	if (directoryPath == NULL)
	{
		// print message
		printf("Out of memory.\n");

		// return failed status code
		return -1;
	}

	// set null terminator at first char in char array to make empty string
	directoryPath[0] = '\0';

	// concatenate drive and directory to make directory path
	::strcat(directoryPath, driveBuffer);
	::strcat(directoryPath, directoryBuffer);

	// print log information message
	printf("Checking if %s already exists...\n", directoryPath);

	// directory path exists
	if (DirectoryExists(directoryPath) == FALSE)
	{
		// print log information message
		printf("Creating directory %s...\n", directoryPath);

		// create directory
		if (::SHCreateDirectoryEx(NULL, directoryPath, NULL) != ERROR_SUCCESS)
		{
			// print message
			PrintOsErrorText();

			// free memory allocated on the heap
			free(directoryPath);

			// return failed status code
			return -1;
		}
	}

	// create fill colorref
	COLORREF fillColor = RGB(red, green, blue);

	// print log information message
	printf("Creating image %s...\n", filePath);

	// create blake image format (.bif)
	if (CreateImage(filePath, pixelWidth, pixelHeight, fillColor) == FALSE)
	{
		// free memory allocated on the heap
		free(directoryPath);

		// return failed status code
		return -1;
	}

	// print log information message
	printf("Successfully created image %s.\n", filePath);

	// print log information message
	printf("Displaying image %s....\n", filePath);

	// display blake image format (.bif)
	if (DisplayImage(filePath) == FALSE)
	{
		// free memory allocated on the heap
		free(directoryPath);

		// return failed status code
		return -1;
	}

	// print log information message
	printf("Successfully displayed image %s.\n", filePath);

	// free memory allocated on the heap
	free(directoryPath);

	// return success status code
    return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Method		CreateImage
//	Purpose:	Creates a new BIF image file
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CreateImage(const char* filePath, unsigned short pixelWidth, unsigned short pixelHeight, COLORREF fillColor)
{
	// validate parameters
	if (filePath == NULL)
	{
		printf("Invalid parameter FilePath NULL.\n");
		return FALSE;
	}

	// get red color
	BYTE red = GetRValue(fillColor);

	// get green color
	BYTE green = GetGValue(fillColor);

	// get blue color
	BYTE blue = GetBValue(fillColor);

	// number of bytes per color channel (our fill color is specified using one byte per color channel so numBytesPerChannel = 1)
	int numBytesPerChannel = 1;

	// number of color channels (bands) in image (rgb = 3)
	int numColorChannels = 3;

	// compute pixel buffer size (raw memory is always allocated using the count of data needed in bytes)
	__int64 pixelBufferSize = pixelWidth * pixelHeight * numBytesPerChannel * numColorChannels;

	// allocate memory buffer on the heap (malloc is the ANSI C way of allocating on the heap, ANSI C++ can also use the "new" keyword,
	// the WIN32 API has even more ways to allocate memory but those are specific to Windows)
	BYTE* pixels = (BYTE*) malloc(pixelBufferSize);
	if (pixels == NULL)
	{
		printf("Failed to allocate pixel buffer.\n");
		return FALSE;
	}

	// zero pixel memory buffer
	::memset(pixels, 0, pixelBufferSize);

	// fill the pixels with the fill color
	BYTE* scan0 = (BYTE*) pixels;
	for (int y = 0; y < pixelHeight; ++y)
	{
		for (int x = 0; x < pixelWidth; ++x)
		{
			scan0[0] = red;
			scan0[1] = green;
			scan0[2] = blue;
			scan0 += 3;
		}
	}

	// create file
	HANDLE file = ::CreateFile(filePath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (file == INVALID_HANDLE_VALUE)
	{
		PrintOsErrorText();
		free(pixels);
		return FALSE;
	}

	// write bif four letter character code (4CC)
	DWORD numberOfBytesWritten = 0;
	if (::WriteFile(file, BifFourCC, (DWORD) sizeof(BifFourCC), &numberOfBytesWritten, NULL) == FALSE)
	{
		PrintOsErrorText();
		free(pixels);
		::CloseHandle(file);
		return FALSE;
	}

	// write file version
	if (::WriteFile(file, &FileVersion, (DWORD) sizeof(unsigned short), &numberOfBytesWritten, NULL) == FALSE)
	{
		PrintOsErrorText();
		free(pixels);
		::CloseHandle(file);
		return FALSE;
	}

	// write pixel width
	if (::WriteFile(file, &pixelWidth, (DWORD) sizeof(pixelWidth), &numberOfBytesWritten, NULL) == FALSE)
	{
		PrintOsErrorText();
		free(pixels);
		::CloseHandle(file);
		return FALSE;
	}

	// write pixel height
	if (::WriteFile(file, &pixelHeight, (DWORD) sizeof(pixelHeight), &numberOfBytesWritten, NULL) == FALSE)
	{
		PrintOsErrorText();
		free(pixels);
		::CloseHandle(file);
		return FALSE;
	}

	// write fill color
	if (::WriteFile(file, &fillColor, (DWORD) sizeof(fillColor), &numberOfBytesWritten, NULL) == FALSE)
	{
		PrintOsErrorText();
		free(pixels);
		::CloseHandle(file);
		return FALSE;
	}

	// write pixels
	if (::WriteFile(file, pixels, (DWORD) pixelBufferSize, &numberOfBytesWritten, NULL) == FALSE)
	{
		PrintOsErrorText();
		free(pixels);
		::CloseHandle(file);
		return FALSE;
	}

	// flush data to disk
	::FlushFileBuffers(file);

	// close file handle
	::CloseHandle(file);

	// free heap memory
	free(pixels);

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Method		DisplayImage
//	Purpose:	Reads a BIF image file and displays it in a window
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL DisplayImage(const char* filePath)
{
	// validate parameters
	if (filePath == NULL)
	{
		printf("Invalid parameter FilePath NULL.\n");
		return FALSE;
	}

	// open file for read only
	HANDLE file = ::CreateFile(filePath, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (file == INVALID_HANDLE_VALUE)
	{
		PrintOsErrorText();
		return FALSE;
	}

	// get file byte size
	DWORD fileByteSize = ::GetFileSize(file, NULL);

	// compute file header byte size - which is the sizeof() each variable that holds the header information ([4CC] + [FileVersion] + [Pixel Width] + [Pixel Height] + [Fill Color] == 14 bytes)
	DWORD fileHeaderByteSize = sizeof(BifFourCC) + sizeof(unsigned short) + sizeof(short) + sizeof(short) + sizeof(COLORREF);

	// validate mimimum file size, which in our case is the size of the header
	if (fileByteSize < fileHeaderByteSize)
	{
		printf("Unsupported or corrupt file. File header must be %lu bytes.\n", fileHeaderByteSize);
		return FALSE;
	}

	// read first four bytes (4CC)
	BYTE fourCC[4] = {};
	DWORD numberOfBytesRead = 0;
	if (::ReadFile(file, fourCC, (DWORD) sizeof(fourCC), &numberOfBytesRead, NULL) == FALSE)
	{
		PrintOsErrorText();
		::CloseHandle(file);
		return FALSE;
	}

	// validate its a BIF1 file by comparing 4 bytes of raw memory
	if (::memcmp(fourCC, BifFourCC, sizeof(fourCC)) != 0)
	{
		printf("Unsupported file type. File doesn't start with correct 4 bytes.\n");
		return FALSE;
	}

	// read file version
	unsigned short fileVersion = 0;
	if (::ReadFile(file, &fileVersion, (DWORD) sizeof(fileVersion), &numberOfBytesRead, NULL) == FALSE)
	{
		PrintOsErrorText();
		::CloseHandle(file);
		return FALSE;
	}

	// validate correct file version for this reader
	if (fileVersion != FileVersion)
	{
		printf("Unsupported file version. This reader only supports version %u.\n", FileVersion);
		return FALSE;
	}

	// read pixel width
	unsigned short pixelWidth = 0;
	if (::ReadFile(file, &pixelWidth, (DWORD) sizeof(pixelWidth), &numberOfBytesRead, NULL) == FALSE)
	{
		PrintOsErrorText();
		::CloseHandle(file);
		return FALSE;
	}

	// read pixel height
	unsigned short pixelHeight = 0;
	if (::ReadFile(file, &pixelHeight, (DWORD) sizeof(pixelHeight), &numberOfBytesRead, NULL) == FALSE)
	{
		PrintOsErrorText();
		::CloseHandle(file);
		return FALSE;
	}

	// read fill color
	COLORREF fillColor = 0;
	if (::ReadFile(file, &fillColor, (DWORD) sizeof(fillColor), &numberOfBytesRead, NULL) == FALSE)
	{
		PrintOsErrorText();
		::CloseHandle(file);
		return FALSE;
	}

	// number of bytes per color channel (our fill color is specified using one byte per color channel so numBytesPerChannel = 1)
	int numBytesPerChannel = 1;

	// number of color channels (bands) in image (rgb = 3)
	int numColorChannels = 3;

	// number of bits per byte
	int numBitsPerByte = 8;

	// number of bits per pixel
	int numBitsPerPixel = numColorChannels * numBytesPerChannel * numBitsPerByte;

	// compute pixel buffer size (raw memory is always allocated using the count of data needed in bytes)
	__int64 pixelBufferSize = pixelWidth * pixelHeight * numBytesPerChannel * numColorChannels;

	// validate file size matches what we want to read out
	if (fileHeaderByteSize + pixelBufferSize > fileByteSize)
	{
		printf("Unsupported or corrupt file. File size must be at least %lu bytes.\n", (DWORD) (fileHeaderByteSize + pixelBufferSize));
		::CloseHandle(file);
		return FALSE;
	}

	// allocate memory buffer on the heap (malloc is the ANSI C way of allocating on the heap, ANSI C++ can also use the "new" keyword,
	// the WIN32 API has even more ways to allocate memory but those are specific to Windows)
	BYTE* pixels = (BYTE*) malloc(pixelBufferSize);
	if (pixels == NULL)
	{
		printf("Failed to allocate pixel buffer.\n");
		::CloseHandle(file);
		return FALSE;
	}

	// zero pixel memory buffer
	::memset(pixels, 0, pixelBufferSize);

	// read pixels
	if (::ReadFile(file, pixels, (DWORD) pixelBufferSize, &numberOfBytesRead, NULL) == FALSE)
	{
		PrintOsErrorText();
		free(pixels);
		::CloseHandle(file);
		return FALSE;
	}

	// close file handle
	::CloseHandle(file);

	// get console window instance handle
	HINSTANCE instance = (HINSTANCE) ::GetModuleHandle(NULL);
	if (instance == NULL) 
	{
		printf("Invalid console window instance handle NULL.\n");
		free(pixels);
		return FALSE;
	}

	// window class name
	const char CLASS_NAME[] = "Blake Window Class";

	// window class
	WNDCLASS wc = {};
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = instance;
	wc.lpszClassName = CLASS_NAME;

	// register window class name
	::RegisterClass(&wc);

	// Create the window.
	HWND hwnd = ::CreateWindowEx(WS_EX_CLIENTEDGE, CLASS_NAME, "Blake Image Format", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, (int) pixelWidth, (int) pixelHeight, NULL, NULL, instance, NULL);
	if (hwnd == NULL)
	{
		printf("Invalid window handle NULL.\n");
		free(pixels);
		return FALSE;
	}

	HICON icon = ::LoadIcon(instance, MAKEINTRESOURCE(IDI_APP_ICON));
	::SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM) icon);
	::SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM) icon);

	// get handle to window device context
	HDC hdc = ::GetDC(hwnd);

	// create bitmap info
	BITMAPINFO bitmapInfo = {};
	bitmapInfo.bmiHeader.biBitCount = numBitsPerPixel;
	bitmapInfo.bmiHeader.biWidth = pixelWidth;
	bitmapInfo.bmiHeader.biHeight = pixelHeight;
	bitmapInfo.bmiHeader.biPlanes = 1;
	bitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);

	// create device independent bitmap (DIB)
	UINT* bits = 0;
	HBITMAP bitmap = ::CreateDIBSection(hdc, (BITMAPINFO*) &bitmapInfo, DIB_RGB_COLORS, (void **)&bits, NULL, 0);

	// copy the pixels to the dib section - rgb to bgr
	BYTE* sourceScan0 = (BYTE*) pixels;
	BYTE* targetScan0 = (BYTE*) bits;
	for (int y = 0; y < pixelHeight; ++y)
	{
		for (int x = 0; x < pixelWidth; ++x)
		{
			targetScan0[0] = sourceScan0[2];
			targetScan0[1] = sourceScan0[1];
			targetScan0[2] = sourceScan0[0];
			sourceScan0 += 3;
			targetScan0 += 3;
		}
	}

	// create memory device context
	::GetObject(bitmap, sizeof(BITMAP), &mBitmapObject);
	mMemoryHdc = ::CreateCompatibleDC(hdc);
	HBITMAP oldBitmap = (HBITMAP) ::SelectObject(mMemoryHdc, bitmap);
	::ReleaseDC(hwnd, hdc);

	// show window
	::ShowWindow(hwnd, SW_SHOW);

	// update window
	::UpdateWindow(hwnd);

	// center window
	CenterWindow(hwnd);

	// set cursor to arrow
	::SetCursor(::LoadCursor(NULL, IDC_ARROW));

	// minimize console window
	::ShowWindow(::GetConsoleWindow(), SW_MINIMIZE);

	// process windows messages
	MSG msg = {};
	while (::GetMessage(&msg, NULL, 0, 0) > 0)
	{
		::TranslateMessage(&msg);
		::DispatchMessage(&msg);
	}
															
	// free heap memory
	free(pixels);

	// delete device context
	DeleteDC(mMemoryHdc);

	// delete bitmap
	::DeleteObject(bitmap);

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Method		WindowProc
//	Purpose:	Windows message call back routine
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_CLOSE:
		{
			DestroyWindow(hwnd);
			break;
		}
		case WM_DESTROY:
		{
			::PostQuitMessage(0);
			break;
		}
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = ::BeginPaint(hwnd, &ps);
			::FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));
			if (mMemoryHdc != NULL)
			{
				::SetStretchBltMode(hdc, STRETCH_HALFTONE);
				::StretchBlt(hdc, 0, 0, mBitmapObject.bmWidth, mBitmapObject.bmHeight, mMemoryHdc, 0, 0, mBitmapObject.bmWidth, mBitmapObject.bmHeight, SRCCOPY);
			}
			::EndPaint(hwnd, &ps);
			break;
		}
		default:
			return ::DefWindowProc(hwnd, uMsg, wParam, lParam);
	}

	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Method		ConfigureScreen
//	Purpose:	Prints usage to the screen
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ConfigureScreen()
{
	// clear the screen
	::system("cls");

	// get current local time from OS
	time_t currentTime = ::time(NULL);
	struct tm* currentLocalTime = ::localtime(&currentTime);
	int currentYear = currentLocalTime->tm_year + 1900; // year is number of years since 1900 so we need to add 1900 for gregorian calendar year

	// window pixel size
	short pixelWidth = 800;
	short pixelHeight = 600;

	// set the console title
	::SetConsoleTitle("Blake Image Format");

	// set console window buffer size
	COORD size;
	size.X = pixelWidth * 2;
	size.Y = pixelHeight * 2;
	::SetConsoleScreenBufferSize(::GetStdHandle(STD_OUTPUT_HANDLE), size);

	// get console window handle
	HWND hwnd = ::GetConsoleWindow();

	// center window and resize
	int desktopWidth = ::GetSystemMetrics(SM_CXFULLSCREEN);
	int desktopHeight = ::GetSystemMetrics(SM_CYFULLSCREEN);
	int left = desktopWidth / 2 - pixelWidth / 2;
	int top = desktopHeight / 2 - pixelHeight / 2;
	::MoveWindow(hwnd, left, top, pixelWidth, pixelHeight, TRUE);

	// set text blue
	::SetConsoleTextAttribute(::GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY);

	// print application information
	printf("Blake Image Format %u\n", FileVersion);
	printf("Copyright (c) %d Blake Hamilton. All rights reserved.\n\n", currentYear);

	// set text white
	::SetConsoleTextAttribute(::GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);

	// print usage message
	printf("Usage Information\n\n");
	printf("Application arguments (required):\n");
	printf("Pixel Width. (range: 1 - 65536)\n");
	printf("Pixel Height. (range: 1 - 65536)\n");
	printf("Red Color Channel. (range: 0 - 255)\n");
	printf("Green Color Channel. (range: 0 - 255)\n");
	printf("Blue Color Channel. (range: 0 - 255)\n");
	printf("Full path to image file. (example: 800 600 255 0 255 \"c:\\images\\image.bif\")\n\n");

	// print notes
	printf("Notes\n\n");
	printf("1. Paths with spaces need to be wrapped in double quotes.\n");
	printf("2. The utility will print log information to the screen.\n\n");

	// set text yellow
	::SetConsoleTextAttribute(::GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);

	// print log header
	printf("APPLICATION LOG:\n");

	// set text white
	::SetConsoleTextAttribute(::GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Method		CenterWindow
//	Purpose:	Centers a window to the parent window
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CenterWindow(HWND hwndWindow)
{
	RECT rectWindow, rectParent;
	HWND hwndParent = ::GetParent(hwndWindow);
	if (hwndParent == NULL)
		hwndParent = ::GetDesktopWindow();
	
	::GetWindowRect(hwndWindow, &rectWindow);
	::GetWindowRect(hwndParent, &rectParent);
	int windowWidth = rectWindow.right - rectWindow.left;
	int windowHeight = rectWindow.bottom - rectWindow.top;
	int screenWidth = ::GetSystemMetrics(SM_CXFULLSCREEN);
	int screenHeight = ::GetSystemMetrics(SM_CYFULLSCREEN);
	int left = screenWidth / 2 - windowWidth / 2;
	int top = screenHeight / 2 - windowHeight / 2;
	return ::MoveWindow(hwndWindow, left, top, windowWidth, windowHeight, TRUE);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Method		PrintUsageError
//	Purpose:	Prints a command line usage error
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PrintUsageError()
{
	// set text red
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_INTENSITY);

	// print error message
	printf("Invalid number of command line parameters.\n");

	// set text white
	::SetConsoleTextAttribute(::GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);

	// print error message
	printf("Parameters are: [Pixel Width] [Pixel Height] [Red Color Channel] [Green Color Channel] [Blue Color Channel] [File Path]\n");
	printf("Example: 800 600 255 0 255 \"c:\\images\\image.bif\"\n\n");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Method		FileExists
//	Purpose:	Returns TRUE if the file path exists
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL FileExists(const char* filePath)
{
	if (filePath == NULL)
	{
		::SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}

	DWORD attributes = ::GetFileAttributes(filePath);
	BOOL result = (attributes != INVALID_FILE_ATTRIBUTES && ((attributes & FILE_ATTRIBUTE_DIRECTORY) == 0)) ? TRUE : FALSE;
	if (result == FALSE) ::SetLastError(ERROR_FILE_NOT_FOUND);

	return result;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Method		DirectoryExists
//	Purpose:	Returns TRUE if the directory path exists
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL DirectoryExists(const char* directoryPath)
{
	if (directoryPath == NULL)
	{
		::SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}

	DWORD attributes = ::GetFileAttributes(directoryPath);
	BOOL result = (attributes != INVALID_FILE_ATTRIBUTES && (attributes & FILE_ATTRIBUTE_DIRECTORY)) ? TRUE : FALSE;
	if (result == FALSE) ::SetLastError(ERROR_PATH_NOT_FOUND);

	return result;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Method		PrintOsErrorText
//	Purpose:	Prints the friendly text associated with the last OS error message numeric code
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PrintOsErrorText()
{
	LPVOID buffer = NULL;
	::FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, ::GetLastError(), 0, (LPTSTR) &buffer, 0, NULL);
	if (buffer == NULL) return;

	printf("%s\n", (char*) buffer);
	::LocalFree(buffer);
}