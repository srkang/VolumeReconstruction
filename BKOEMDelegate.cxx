// This file is still under debugging and optimization.
#include "BKOEMDelegate.h"

BKOEMDelegate::BKOEMDelegate(bool crc, bool ack)
	:crc(crc),
	ack(ack),
	m_Image(NULL),
	m_ImageSize(cvSize(0,0)),
	m_Streaming(false)
{
	this->m_ImageSize = cvSize(-1,-1);
	memset((void*)&m_bkImageHeader, 0, sizeof(BKImageHeader));
	// Instantiate a CircularBuffer for data streaming
	this->buf = new CircularBuffer(BKOEMDelegate::MAX_IMAGE_BUFFER_SIZE);
	// Instantiate a ClientSocket for communication with the BK device.
	// The socket is used for both data streaming and message communication.
	socket = igtl::ClientSocket::New();
}

BKOEMDelegate::~BKOEMDelegate()
{
	delete this->buf;
	if (socket) {
		socket->CloseSocket();
		socket = NULL;
	}
}

// Contineously receive the frame grabbing images
// and data messages with data subscribed for
void BKOEMDelegate::run()
{
	char ch = 0;
	int bytes_received = 0;

	char* tmp = new char[DEFAULT_COLOR_IMAGE_BUFFER_SIZE];

	bool esc = false;  // ESC detected flag
	bool soh = false;  // Block Header detected flag
	bool eot = false;  // Block Terminator detectedn flag

	while (this->isRunning()) 	{
		if (this->isStreaming()) {
			// Receive and dump into the circular buffer
			bytes_received = this->socket->Receive(tmp, DEFAULT_IMAGE_BUFFER_SIZE, 0);
			if (bytes_received)
				if (!this->buf->push(tmp, bytes_received))
					std::cout << "Cannot push data!\n";

			// The previous datum (char) is neither SOH nor EOT
			// and the current datum (char) is pop correctly,
			while (!soh && !eot && this->buf->pop(&ch)) 	{
				// if escape character detected, invert the character before it.
				if (esc) {
					esc = false;
					if ( m_MessageType == mtDATA && m_DataMessageType == dmGrabFrame)
						this->m_bkImageHeader.push(~ch); // invert the character
					else
						std::cout << ~ch << std::endl;
					continue;
				}

				// Detect reserved characters
				switch (ch) {
				case BKImageHeader::ESC:  // escape character
					esc = true;
					break;
				case BKImageHeader::SOH:  // block header
					soh = true;
					break;
				case BKImageHeader::EOT:	// block terminator
					eot = true;
					break;
				default:
					// If processing frame grabbing image,
					// push the data into the image header for later use
					if ( m_MessageType == mtDATA && m_DataMessageType == dmGrabFrame)
						this->m_bkImageHeader.push(ch);
				}
			}

			/* 
				When EOT (block terminator) is detected,
				parse the data according the Program Message Format.
				NOTE:
				ONLY a grabed frame is expected in the Program Message,
				and NO any other Program Messages will be processed here.
			*/
			if (eot) {
				eot = false;
				if ( m_MessageType == mtDATA ) {
					if ( m_DataMessageType == dmGrabFrame )
						this->grabFrame();
					else
						std::cout << m_MessageType << ":" << m_DataMessageType << std::endl;
				}
			} // end if (eot)

			// If SOH (Block Header) is met, ...
			// We need to distinguish what type the message is.
			if (soh) {
				soh = false;
				m_MessageType = parseMessageType(this->buf->getHead());
				switch ( m_MessageType ) {
				case mtDATA:
				case mtSDATA:
					processMessageData();
					break;
				case mtERROR:
					//processError();
					break;
				case mtEVENT:
					//processEvent();
					break;
				case mtACK:
					// confirmation to the lastest command/query
					break;
				}
			} // end if (soh)
		}
	}
}

// Extract the image from the Message data
bool BKOEMDelegate::grabFrame()
{
	bool Result = true;

	//std::cout << "BKOEMDelegate threadID= " << this->currentThreadId() << std::endl;
	
	this->m_bkImageHeader.UpdateTimeStamp();		// ?
	this->CurTimeStamp = this->m_bkImageHeader.GetTimeStamp();
	
	// check the time to grab frame
	//clock_t t;
	//t = clock();
	
	if (m_bkImageHeader.GetDataSize() == m_Image->memorySize()) 	{
		/* Color Doppler image */
		memcpy(	this->m_Image->getImage(cnmcfusion::LEFT).data, 
						this->m_bkImageHeader.getDataPointer(), 
						this->m_bkImageHeader.GetDataSize() );
		// Convert BGR to RGB format. Note that, ultrasound only have one "camera"
		cv::cvtColor(this->m_Image->getImage(cnmcfusion::LEFT), this->m_Image->getImage(cnmcfusion::LEFT), CV_BGR2RGB);

		//// added by Jihun
		//memcpy( this->m_Image->getImageVec3b().data(), 
		//				this->m_bkImageHeader.getDataPointer(), 
		//				3*m_ImageSize.width*m_ImageSize.height);

	} else if (m_bkImageHeader.GetDataSize() == m_Image->memorySize()/3) {
		/* gray-scale image */
		// Convert gray image to RGB image
		IplImage *imgheader = cvCreateImageHeader(this->m_ImageSize, IPL_DEPTH_8U , 1);
		imgheader->imageData = (char*)this->m_bkImageHeader.getDataPointer();
		// The conversion implicitly copies the data
		cvConvertImage(imgheader, &(IplImage(this->m_Image->getImage(cnmcfusion::LEFT))));
		cvReleaseImageHeader(&imgheader);
		
		//char* m2 = new char[3*m_ImageSize.width*m_ImageSize.height];
		//memcpy (m2, this->m_Image->getImage().data, m_ImageSize.width*m_ImageSize.height);
		//this->m_Image->getImageVec3b().reserve(630000);
		//this->m_Image->getImageVec3b().erase(this->m_Image->getImageVec3b().begin(), this->m_Image->getImageVec3b().end());
		//this->m_Image->getImageVec3b().clear();
		//this->m_Image->getImageVec3b().assign((char*)this->m_bkImageHeader.getDataPointer2(), (char*)this->m_bkImageHeader.getDataPointer2()+this->m_Image->getImage().rows*this->m_Image->getImage().cols);
		
		//printf(" matrix a[0]: \n");
		//int count = 0;
		//for( size_t i = 0; i < m_ImageSize.height*m_ImageSize.width; i++ ) {
		//		// observe the type used in the template
		//		printf( " %d  ", this->m_Image->getImageVec3b().at(i));
		//		count++;
		//		if (count == m_ImageSize.width){
		//			printf("\n");
		//			count = 0;
		//		}
		//}


		//cv::Mat B2;
		//B2.reshape;

		//B2 = this->m_Image->getImage().reshape(3, m_ImageSize.width*m_ImageSize.height);
		//this->m_Image->getImageVec3b() = cv::Mat_<cv::Vec3b>(B2);


		// Why this does not work??
		//cv::Mat temp(this->m_ImageSize, CV_8U, this->m_bkImageHeader.getDataPointer());
		//imwrite("temp.bmp", temp);
		//temp.convertTo( this->m_StereoImage->getImage(LEFT), 
		//								this->m_StereoImage->getImage(LEFT).type() );
		//imwrite("converted.bmp", this->m_StereoImage->getImage(LEFT));
	} else {
		std::cout << "Unknown image size/format.\n";
		Result = false;
	}

	if ( Result )
	{
		//this->m_bkImageHeader.UpdateTimeStamp();		// ?
		//this->CurTimeStamp = this->m_bkImageHeader.GetTimeStamp();
		this->m_bkImageHeader.ResetData();					// reset the header
	}

	//std::cout << "Grab time = " << clock() - t << std::endl;
	 
	return Result;
}

void BKOEMDelegate::processMessageHead()
{
}

// Deal with the Program Message data
void BKOEMDelegate::processMessageData()
{
	int len;
	char* dummy;
	// determing which type of the message is
	m_DataMessageType = parseDataMessageType(this->buf->getHead());
	switch ( m_DataMessageType )
	{
	case dmGrabFrame:
		// the received Program Message is a frame grabbing image.
		this->buf->pop((char*)&m_bkImageHeader.primary, sizeof(BKImageHeader::Primary), true);
		m_bkImageHeader.UpdateN();
		this->buf->pop((char*)m_bkImageHeader.secondary.dataSize, m_bkImageHeader.GetN(), true);
		m_bkImageHeader.UpdateDataSize();
		m_bkImageHeader.SetData();
		break;
	case dmWinSize:
		if ( m_MessageType == mtDATA )
			sscanf(this->buf->getHead(), "DATA:US_WIN_SIZE %d,%d;", &m_ImageSize.width, &m_ImageSize.height);
		else if ( m_MessageType == mtSDATA)
			sscanf(this->buf->getHead(), "SDATA:US_WIN_SIZE %d,%d;", &m_ImageSize.width, &m_ImageSize.height);
		// pop out the data
		len = strchr(this->buf->getHead(), ';') - this->buf->getHead() + 1;
		dummy = (char *)malloc(len);
		this->buf->pop(dummy, len, true);
		std::cout << "Image size changed: " << m_ImageSize.width << std::endl;
		break;
	case dmGeometryScanArea:
		float val[8];
		if ( m_MessageType == mtDATA )
			sscanf( this->buf->getHead(), "DATA:B_GEOMETRY_SCANAREA:A %f,%f,%f,%f,%f,%f,%f,%f;", 
							&val[0], &val[1], &val[2], &val[3], &val[4], &val[5], &val[6], &val[7] );
		else if ( m_MessageType == mtSDATA)
			sscanf( this->buf->getHead(), "SDATA:B_GEOMETRY_SCANAREA:A %f,%f,%f,%f,%f,%f,%f,%f;", 
							&val[0], &val[1], &val[2], &val[3], &val[4], &val[5], &val[6], &val[7] );
		// pop out the data
		len = strchr(this->buf->getHead(), ';') - this->buf->getHead() + 1;
		dummy = (char *)malloc(len);
		this->buf->pop(dummy, len, true);
		this->m_PixelSize = 1000.0*(val[7] - val[3])/this->m_ImageSize.height;
		std::cout << "Pixel size changed: " << this->m_PixelSize << std::endl;
		break;
	}
}

// Return the program message type in the buffer
MessageType BKOEMDelegate::parseMessageType(const char* buf)
{
	if (!strncmp(buf, "DATA", 4))
		return mtDATA;
	else if (!strncmp(buf, "SDAT", 4))
		return mtSDATA;
	else if (!strncmp(buf, "STAT", 4))
		return mtSTATUS;
	else if (!strncmp(buf, "ERRO", 4))
		return mtERROR;
	else if (!strncmp(buf, "EVEN", 4))
		return mtEVENT;
	else if (!strncmp(buf, "ALIV", 4))
		return mtALIVE;
	else if (!strncmp(buf, "ACK;", strlen("ACK;")))
		return mtACK;
	else
		return mtUnknown;
}

// Return the type of the data message in the buffer.
// The message is either queried or subscribed.
DataMessageType BKOEMDelegate::parseDataMessageType(const char* buf)
{
	if (!strncmp(buf, "DATA:GRAB_FRAME #", strlen("DATA:GRAB_FRAME #")))
		return dmGrabFrame;
	else if ( !strncmp(buf, "DATA:US_WIN_SIZE", strlen("DATA:US_WIN_SIZE")) ||
						!strncmp(buf, "SDATA:US_WIN_SIZE", strlen("SDATA:US_WIN_SIZE")) )
		return dmWinSize;
	else if ( !strncmp(buf, "DATA:B_GEOMETRY_PIXEL", strlen("DATA:B_GEOMETRY_PIXEL")) ||
						!strncmp(buf, "SDATA:B_GEOMETRY_PIXEL", strlen("SDATA:B_GEOMETRY_PIXEL")) )
		return dmGeometryPixel;
	else if ( !strncmp(buf, "DATA:B_GEOMETRY_TISSUE", strlen("DATA:B_GEOMETRY_TISSUE")) ||
						!strncmp(buf, "SDATA:B_GEOMETRY_TISSUE", strlen("SDATA:B_GEOMETRY_TISSUE")) )
		return dmGeometryTissue;
	else
		return dmUnknown;
}

// Connect to the BK ultrasound via TCP @ port and 
// If success, get the image size and pixel size,
// and then instantiate a cnmcUS3DImage object for saving the frame grabbing image.
cnmcStereoImage* BKOEMDelegate::Connect(const std::string addr, const int port)
{
	std::cout << "Connecting to BK Ultrasound ...\n  Address: " << addr << "@" << port << std::endl;
	if (!this->socket->ConnectToServer(addr.c_str(), port)) 	{
		this->QueryImageSize(false);  // subscribe the US_WIN_SIZE
		this->QueryPixelSize(false);  // subscribe the B_GEOMETRY_SCANAREA
		// Input Ultrasound image: grayscale or color (default)
		if (!this->m_Image){
			this->m_Image = new cnmcStereoImage(this->m_ImageSize, 3, false);  // initialize as color image
		}
		this->start();
		
	}
	return this->m_Image;
}

bool BKOEMDelegate::Disconnect()
{
	this->terminate();
	// Wait untill it is terminated.
	while (this->isRunning()) sleep(1);
	socket->CloseSocket();
	return true;
}

/* 
	 Query the scanner for data
*/

// Start frame grabbing using QUERY:GRAB_FRAME "ON";
void BKOEMDelegate::StartStream(float fps)
{
	char str[BKOEMDelegate::DEFAULT_QUERY_SIZE] = "QUERY:GRAB_FRAME \"ON\",%d;";
	char cmd[128];

	// Terminate the thread if it is running
	// to ensure the received Message will not lose
	if ( this->isRunning() )
	{
		this->terminate();
		while ( this->isRunning() ) sleep(1);
	}

	sprintf(cmd, str, int(fps));
	// Encode the Message,
	int len = this->packSendBuffer(cmd, strlen(cmd));
	// prepare the receiving buffer
	memset(m_rcvBuffer, 0, DEFAULT_COLOR_IMAGE_BUFFER_SIZE);
	// and send the Message to BK ultrasound machine.
	socket->Send(m_sndBuffer, len);
	// Receive the feedback ACK message but ignore it
	socket->Receive(m_rcvBuffer, BKOEMDelegate::DEFAULT_DATA_SIZE, 0);
	if ( !this->Acknowledged() )
		std::cout << "Do not get expected ACK;" << std::endl;
	// Finally, set the internal flag.
	this->m_Streaming = true;

	// Start the background socket communication thread again
	this->start();
}

// Stop frame grabbing using QUERY:GRAB_FRAME "OFF";
void BKOEMDelegate::StopStream()
{
	int k = 0; //, j;
	char str[BKOEMDelegate::DEFAULT_QUERY_SIZE] = "QUERY:GRAB_FRAME \"OFF\";";

	// Terminate the thread if it is running
	// to ensure the received Message will not lose
	if ( this->isRunning() )
	{
		this->terminate();
		while ( this->isRunning() ) sleep(1);
	}
	// Send command to the device
	int len = this->packSendBuffer(str, strlen(str));
	socket->Send(m_sndBuffer, len);
	/////////////////////////////////////////////////////////////////
	// Why NO "ACK;"?
	// This is different from QUERY:GRAB_FRAME "ON"
	/////////////////////////////////////////////////////////////////
	//do
	//{
	//	this->socket->Receive(m_rcvBuffer, DEFAULT_IMAGE_BUFFER_SIZE);
	//} 
	//while ( !this->Acknowledged() );
	// Just to empty whatever left in the buffer
	memset(m_rcvBuffer, 0, DEFAULT_COLOR_IMAGE_BUFFER_SIZE);

	// Set the internal flag once all works have been done.
	this->m_Streaming = false;
}

// QUERY:US_WIN_SIZE;
void BKOEMDelegate::QueryImageSize(bool QueryOnly)
{
	int len;
	char str[BKOEMDelegate::DEFAULT_DATA_SIZE];
	if ( QueryOnly )
		len = this->packSendBuffer("QUERY:US_WIN_SIZE;", strlen("QUERY:US_WIN_SIZE;"));
	else
		len = this->packSendBuffer("CONFIG:DATA:SUBSCRIBE \"US_WIN_SIZE\";", 
																strlen("CONFIG:DATA:SUBSCRIBE \"US_WIN_SIZE\";"));

	socket->Send(m_sndBuffer, len);
	// The current value will be returned IMMEDIATELY after having been subscribed.
	len = socket->Receive(m_rcvBuffer, 50, 0);
	this->unpackRceivedBuffer(str, len);
	if ( !QueryOnly )
	{
		// In addition, an "ACK;" (acknowledgement) is expected to return from the device
		this->socket->Receive(m_rcvBuffer, DEFAULT_IMAGE_BUFFER_SIZE, 0);
		if ( !this->Acknowledged() )
			std::cout << "Did not get expected ACK;" << std::endl;
		// parse the string
		sscanf(str, "SDATA:US_WIN_SIZE %d,%d;", &m_ImageSize.width, &m_ImageSize.height);
		std::cout << "  Image size: " << m_ImageSize.height << "x" << m_ImageSize.width << std::endl;
	}

	//if ( QueryOnly )
	//	sscanf(str, "DATA:US_WIN_SIZE %d,%d;", &m_ImageSize.width, &m_ImageSize.height);
}

// QUERY:B_GEOMETRY_SCANAREA:A
double BKOEMDelegate::QueryPixelSize(bool QueryOnly)
{
	int len;
	char str[BKOEMDelegate::DEFAULT_DATA_SIZE];

	if ( this->m_ImageSize.width < 0 || this->m_ImageSize.height < 0 )
		this->QueryImageSize(QueryOnly);

	if ( QueryOnly )
		len = this->packSendBuffer( "QUERY:B_GEOMETRY_SCANAREA:A;", strlen("QUERY:B_GEOMETRY_SCANAREA:A;"));
	else
		len = this->packSendBuffer( "CONFIG:DATA:SUBSCRIBE \"B_GEOMETRY_SCANAREA\";", 
																strlen("CONFIG:DATA:SUBSCRIBE \"B_GEOMETRY_SCANAREA\";") );
	socket->Send(m_sndBuffer, len);
	memset(m_rcvBuffer, 0, BKOEMDelegate::DEFAULT_DATA_SIZE);
	len = socket->Receive(m_rcvBuffer, BKOEMDelegate::DEFAULT_DATA_SIZE, 0);
	this->unpackRceivedBuffer(str, len);

	float val[8];
	if ( !QueryOnly )
	{
		// In addition, an "ACK;" (acknowledgement) is expected to return from the device
		this->socket->Receive(m_rcvBuffer, DEFAULT_DATA_SIZE, 0);
		if ( !this->Acknowledged() )
			std::cout << "Did not get expected ACK;" << std::endl;
		sscanf( str, "SDATA:B_GEOMETRY_SCANAREA:A %f,%f,%f,%f,%f,%f,%f,%f;", 
						&val[0],&val[1],&val[2],&val[3],&val[4],&val[5],&val[6],&val[7] );
		std::cout << "  Start Depth: " << val[3] << "\n  Stop Depth: " << val[7] << std::endl;
		this->m_PixelSize = 1000.0*(val[7] - val[3])/this->m_ImageSize.height;
		std::cout << "  Pixel size: " << this->m_PixelSize << std::endl;
	}
	//else
	////if ( QueryOnly )
	//	sscanf( str, "DATA:B_GEOMETRY_SCANAREA:A %f,%f,%f,%f,%f,%f,%f,%f;", 
	//					&val[0],&val[1],&val[2],&val[3],&val[4],&val[5],&val[6],&val[7] );

	return this->m_PixelSize;
}

// QUERY:TRANSDUCER_LIST;
//void BKOEMDelegate::QueryTransducer()
//{
//	char cmd[128] = "QUERY:TRANSDUCER:A;";
//	int len = this->packSendBuffer(cmd, strlen(cmd));
//	socket->Send(m_sndBuffer, len);
//	len = socket->Receive(m_rcvBuffer, 50, 0);
//	this->unpackRceivedBuffer(cmd, len);
//	sscanf(cmd, "DATA:TRANSDUCER:A \"%d\",\"%d\";", &m_TransducerConnectPort, &m_TransducerID);
//	std::cout << "Transducer  " << m_TransducerID << " on " << m_TransducerConnectPort << std::endl;
//}

// QUERY:B_FRAMERATE:A;
float BKOEMDelegate::GetFrameRate()
{
	float fr = 0.0;
	char cmd[BKOEMDelegate::DEFAULT_QUERY_SIZE] = "QUERY:B_FRAMERATE:A;";
	int len = this->packSendBuffer(cmd, strlen(cmd));
	socket->Send(m_sndBuffer, len);
	len = socket->Receive(m_rcvBuffer, BKOEMDelegate::DEFAULT_DATA_SIZE, 0);
	this->unpackRceivedBuffer(cmd, len);
	sscanf(cmd, "DATA:B_FRAMERATE:A %f;", &fr);
	return fr;
}

/* 
	 Send commands to the ultrasound device 
*/

bool BKOEMDelegate::freeze()
{
	char str[BKOEMDelegate::DEFAULT_QUERY_SIZE] = "COMMAND:SCAN \"FREEZE\";";
	int len = this->packSendBuffer(str, strlen(str));
	socket->Send(m_sndBuffer, len);
	socket->Receive(str, BKOEMDelegate::DEFAULT_DATA_SIZE, 0);
	return true;
}

// 
bool BKOEMDelegate::scan()
{
	char str[BKOEMDelegate::DEFAULT_QUERY_SIZE] = "COMMAND:SCAN \"SCAN\";";
	int len = this->packSendBuffer(str, strlen(str));
	socket->Send(m_sndBuffer, len);
	socket->Receive(str, BKOEMDelegate::DEFAULT_DATA_SIZE, 0);
	return true;
}

bool BKOEMDelegate::toggle()
{
	char str[BKOEMDelegate::DEFAULT_QUERY_SIZE] = "COMMAND:SCAN \"TOGGLE\";";
	int len = this->packSendBuffer(str, strlen(str));
	socket->Send(m_sndBuffer, len);
	socket->Receive(m_rcvBuffer, BKOEMDelegate::DEFAULT_DATA_SIZE, 0);
	return true;
}

/*
	 Auxilary functions for pack and unpack Messages
	 These functions are used for sending Command and Query Messages.
	 They are not used for frame grabbing.
*/

// Packs the string according to the BK OEM format and puts it in the m_sndBuffer
int BKOEMDelegate::packSendBuffer(char str[], int len)
{
	int n = 0;
	m_sndBuffer[n++] = BKImageHeader::SOH;
	for (int k=0;k<len;k++)
	{
		if ((str[k] == BKImageHeader::SOH)||
			(str[k] == BKImageHeader::EOT)||
			(str[k] == BKImageHeader::ESC))
		{
			m_sndBuffer[n++] = BKImageHeader::ESC;
			m_sndBuffer[n++] = ~(str[k]);
		}
		else
			m_sndBuffer[n++] = str[k];
	}
	m_sndBuffer[n++] = BKImageHeader::EOT;

	// Return the length of the packed string.
	return n;
}

// Unpacks the m_rcvBuffer according to the BK OEM format and puts it in the str
int BKOEMDelegate::unpackRceivedBuffer(char str[], int len)
{
	int n = 0, k = 0;

	// Check for the header character
	if (m_rcvBuffer[k++] != BKImageHeader::SOH)
		return 0;

	for (k=1;k<(len-1);k++)
	{
		if (m_rcvBuffer[k] == BKImageHeader::ESC)
		{
			str[n] = ~m_rcvBuffer[++k];
			if (str[n] == BKImageHeader::EOT)
			{
				
				str[n] = 0;
				return n;
			}
			else
				n++;
		}
		else
			str[n++] = m_rcvBuffer[k];
	}

	str[n] = 0;
	// Return the length of the packed string.
	return n;
}

bool BKOEMDelegate::SendScript(char* cmd)
{
	char str[BKOEMDelegate::DEFAULT_DATA_SIZE];
	int len = this->packSendBuffer(cmd, strlen(cmd));
	socket->Send(m_sndBuffer, len);
	len = socket->Receive(m_rcvBuffer, BKOEMDelegate::DEFAULT_DATA_SIZE, 0);
	this->unpackRceivedBuffer(str, len);
	std::cout << str << std::endl;
	return true;
}

bool BKOEMDelegate::Acknowledged()
{
	char str[BKOEMDelegate::DEFAULT_DATA_SIZE];
	this->unpackRceivedBuffer(str, 6);  // the package length of "ACK;" is 6 
	if (!strncmp(str, "ACK;", 4))
		return true;
	else
		return false;
}

