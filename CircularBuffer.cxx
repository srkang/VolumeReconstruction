#include "CircularBuffer.h"

CircularBuffer::CircularBuffer(int capacity)
	:capacity(capacity)
{
	buffer = new char[capacity];
	// Check if memory was allocated.
	if(buffer == NULL)
		return;

	buffer_end = buffer + capacity;
	// There is no initial data
	count = 0;

	// both head and tail are at the start of the buffer, means no data available.
	head = buffer;
	tail = buffer;

	mutex = new QMutex;
}

CircularBuffer::~CircularBuffer()
{
	delete buffer;
	buffer = buffer_end = head = tail = NULL;

	delete mutex;
}

// Push a whole block of data
bool CircularBuffer::push(const char *data, int len)
{
	mutex->lock();
	for (int k=0;k<len;k++)
		if (!push(data+k))
			return false;
	mutex->unlock();

	return true;
}

bool CircularBuffer::push(const char *data)
{
	// The buffer is full, there is no more space.
	if(count == capacity)
		return false; 

	// Copy len bytes from the item to the circular buffer.
	memcpy(tail, data, 1);

	tail = tail + 1;
	// Go back to the start if already at the end.
	if(tail == buffer_end)
		tail = buffer;
	count++;

	return true;
}

// Pop a whole block of data
bool CircularBuffer::pop(char* data, int len, bool wait)
{
	for (int k=0; k<len; k++)
		if ( !pop(data+k, wait) )
			return false; // Return false if there is no data available

	return true;
}

bool CircularBuffer::pop(char* data, bool wait)
{
	// Return NULL if the buffer is empty
	if (wait)
		while (count == 0) {}
	else if (count == 0)
		return false;

	if (wait)
		mutex->lock();
	else if (!mutex->tryLock())
		return false;

	memcpy(data, head, 1);

	head = head+1;
	// Go back to the start if already at the end.
	if(head == buffer_end)
		head = buffer;
	count--;
	mutex->unlock();

	return true;
}

int CircularBuffer::GetSize()
{
	return count;
}