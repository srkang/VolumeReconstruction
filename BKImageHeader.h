#ifndef BKIMAGEHEADER_H
#define BKIMAGEHEADER_H

#include <iostream>
#include <sstream>
#include <string>

#include "cnmcStereoImage.h"
#include "CircularBuffer.h"

#define LIMIT(x, MIN, MAX)	(x = ((x) < MIN) ? MIN : (( (x) > MAX) ? MAX : (x)) )

enum MessageType {
	mtDATA,		// data messages as reply to queries
	mtSDATA,	// data messages with data subscribed for
	mtSTATUS,	// satus data messages
	mtERROR,	// error messages
	mtEVENT,	// event messages
	mtALIVE, 	// alive messages
	mtACK,		// acknowledge 
	mtUnknown	// unknown Program Message Type
};

enum DataMessageType {
	dmGrabFrame,			// frame grabbing data
	dmWinSize,				// US window size (in pixels)
	dmImageMode,			// imaging mode
	dmGeometryScanArea,
	dmGeometryPixel,	// B-mode view pixel area in screen coordinates
	dmGeometryTissue,	// the tissue area of the window (in meters)
	dmUnknown
};

enum ImageMode {
	imB						 = 0x0001,
	imCFM					 = 0x0002,
	imPowerDoppler = 0x0004,
	imDoppler			 = 0x0008,
	imCW					 = 0x0010,
	imM						 = 0x0020
};

class BKImageHeader
{
public:
	static const char SOH	= 0x01; // ASCII 1, block header
	static const char ESC	= 0x1B; // ASCII 27, escape character
																// The escape character	ESC is inserted
																// before the character and the character itself is inverted.
	static const char EOT	= 0x04; // ASCII 4, block terminator
	BKImageHeader(){};
	~BKImageHeader(){};

	static const int TIMESTAMP_SIZE = 4;
	static const int MAX_DATASIZE_BYTES = 16;
private:
	// The primary section of header with fixed size.
	struct Primary
	{
		// The lead part
		char	lead[sizeof("DATA:GRAB_FRAME #")-1];
		// Number of bytes of packet size
		char	N;
	} primary;

	// Number of bytes of packet size.
	int N;

	// Secondary section of header, size depends on data size.
	struct Secondary
	{
		// n bytes of packet size
		char	dataSize[MAX_DATASIZE_BYTES];
		// 4 bytes of timestamp
		union
		{
			char	str[TIMESTAMP_SIZE];
			int		val;
		} timeStamp;
	} secondary;

	// The integer value of data size = atoi(secondary.dataSize);
	int		dataSize;
	// Received packet data
	char*	data;

	int count;

public:
	friend class BKOEMDelegate;
	//friend class BKImageDecoder;

	void UpdateN(void);
	int GetN(void);

	void UpdateDataSize();
	int GetDataSize();

	void UpdateTimeStamp();
	int GetTimeStamp();

	void ResetData();
	void SetData();

	void push(char ch);
	// Pushing an image out
	void* getDataPointer();

};

#endif //BKIMAGEHEADER_H