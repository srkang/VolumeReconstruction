#ifndef CIRCULAR_BUFFER_H
#define CIRCULAR_BUFFER_H

#include <iostream>
#include <sstream>
#include <string>
#include <QMutex>

class __declspec( dllexport ) CircularBuffer
{
private:
  char	*buffer;			// data buffer
  char	*buffer_end;	// end of data buffer
  int		capacity;			// maximum number of items in the buffer
  int		count;				// number of items in the buffer

  char	*head;				// pointer to head
  char	*tail;				// pointer to tail

	// To avoid collisions and deadlocks
	QMutex	*mutex;
public:
	CircularBuffer(int capacity);
	~CircularBuffer();

	inline bool	push(const char *data, int len);
	// push a single data unit (byte). inline function used to increase efficiency.
	inline bool	push(const char *data);
	
	inline bool	pop(char* data, int len, bool wait = false);
	// pop a single data unit (byte). inline function used to increase efficiency.
	inline bool	pop(char* data, bool wait=false);

	int		GetSize();
	char* getHead() const { return this->head; };
};

#endif //CIRCULAR_BUFFER_H