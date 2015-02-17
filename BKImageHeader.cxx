#include "BKImageHeader.h"

void BKImageHeader::UpdateN(void)
{
	this->N = atoi(&this->primary.N);
	LIMIT(this->N, 0, 15);
}

int BKImageHeader::GetN(void)
{
	return this->N;
}

void BKImageHeader::UpdateDataSize()
{
	this->secondary.dataSize[this->N] = 0;
	this->dataSize = atoi(this->secondary.dataSize);
	if (this->dataSize <= 0)
	{
		std::cout << "Wrong data size!\n";
		this->dataSize = 0;
	}
}

void BKImageHeader::UpdateTimeStamp()
{
	memcpy(this->secondary.timeStamp.str, this->data, TIMESTAMP_SIZE);
}

int BKImageHeader::GetDataSize()
{
	return this->dataSize - TIMESTAMP_SIZE;
};

int BKImageHeader::GetTimeStamp()
{
	return this->secondary.timeStamp.val;
}

void BKImageHeader::ResetData()
{
	this->count = 0;
}

// When the data size (in bytes) of the grabbed image is changed,
// re-allocate a new memory and delete the previous one.
void BKImageHeader::SetData()
{
	static int oldSize=0;
	if (this->dataSize && (this->dataSize != oldSize) )
	{
		oldSize = this->dataSize;
		if (this->data)
			delete this->data;
		this->data = new char[this->dataSize];
		std::cout << "Memory re-allocated!\n";
	}
}

void BKImageHeader::push(char ch)
{
	if (!this->data) return;
	if (count < this->dataSize)
	{	
		this->data[count++] = ch;
	}
}

// return pointer to the latest image
void* BKImageHeader::getDataPointer()
{
	return this->data + TIMESTAMP_SIZE;
}

