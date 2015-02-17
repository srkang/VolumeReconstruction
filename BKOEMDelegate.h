#ifndef _BKOEM_H_
#define _BKOEM_H_

#include <QObject>
#include <QThread>
#include <cv.h>
#include <highgui.h>
#include <igtlClientSocket.h>

#include "cnmcStereoImage.h"
#include "CircularBuffer.h"
#include "BKImageHeader.h"

class __declspec( dllexport ) BKOEMDelegate: public QThread
{
	Q_OBJECT
public:
	BKOEMDelegate(bool crc=false, bool ack=false);
	~BKOEMDelegate();

	// Change the scanniong mode
	bool freeze();
	bool scan();
	bool toggle();

	// Query
	bool SendScript(char* cmd);

	//void	 QueryTransducer();
	void	 QueryImageSize(bool QueryOnly=true);
	double QueryPixelSize(bool QueryOnly=true);
	float  GetFrameRate(void);

	// Start grabbing at a given frame rate
	void StartStream(float fps);
	void StopStream();

	double GetPixelSize() const { return this->m_PixelSize; };
	CvSize GetImageSize() const { return this->m_ImageSize; };
	void	 getImageSize(int &w, int &h) { 
		w = this->m_ImageSize.width; 
		h = this->m_ImageSize.height; 
	};

	// Connect-Disconnect
	cnmcStereoImage* Connect(const std::string, const int);
	bool Disconnect();

	bool isStreaming() const { return this->m_Streaming; };

	int GetTimeStamp() {return this->CurTimeStamp; };


public:
	// Constants
	// Default sizes for Query commands and returned data (except for image data).
	static const int DEFAULT_QUERY_SIZE				= 256;
	static const int DEFAULT_DATA_SIZE				= 256;

	static const int DEFAULT_IMAGE_WIDTH			= 680;
	static const int DEFAULT_IMAGE_HEIGHT			= 660;
	static const int DEFAULT_IMAGE_BUFFER_SIZE		= DEFAULT_IMAGE_WIDTH*DEFAULT_IMAGE_HEIGHT;
	static const int DEFAULT_COLOR_IMAGE_BUFFER_SIZE= DEFAULT_IMAGE_WIDTH*DEFAULT_IMAGE_HEIGHT*3;
	static const int MAX_IMAGE_BUFFER_SIZE			= DEFAULT_IMAGE_WIDTH*DEFAULT_IMAGE_HEIGHT*3*2; // Max buffer size assuming a color image. Max size is approximately 2.57MB.

	



private:
	// CRC/ACK setting: these should match the seetings on the BK system, otherwise the interface will not work.
	bool crc;
	bool ack;
	bool m_Streaming;

	// circular buffer used to store frame grabbing images
	CircularBuffer *buf;
	// Current message type and data message type
	MessageType m_MessageType;
	DataMessageType m_DataMessageType;
	// Return the type of the Program Massage in the buffer
	MessageType parseMessageType(const char*);
	DataMessageType parseDataMessageType(const char*);

	void processMessageHead();
	void processMessageData();
	bool grabFrame();
	// BK specific image header
	BKImageHeader m_bkImageHeader;

	// color image buffer
	char imgBuffer[DEFAULT_COLOR_IMAGE_BUFFER_SIZE];

	cnmcStereoImage* m_Image;
	// added by Jihun

	double m_PixelSize;
	CvSize m_ImageSize;

	void run();

	/*
		The variables and functions below 
		are used for config/command/query Message and 
		are NOT used for frame grabbing data streaming.
	*/
	igtl::ClientSocket::Pointer socket;
	// Packing a message into m_sndBuffer according to the BK OEM format.
	int packSendBuffer(char str[], int len);
	// Unpacking a message from m_rcvBuffer according the BK OEM format.
	int unpackRceivedBuffer(char str[], int len);

	bool Acknowledged();

	// Send/Receive buffers, size should be set to the maximum required size.
	char m_sndBuffer[DEFAULT_QUERY_SIZE];
	char m_rcvBuffer[DEFAULT_COLOR_IMAGE_BUFFER_SIZE];
	
	//char m_TransducerID[4],
	//		 m_TransducerConnectPort;

	int CurTimeStamp;

};

#endif // _BKOEM_H_