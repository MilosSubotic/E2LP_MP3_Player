/**
 * @file LockFreeFifo.c
 * @date Dec 14, 2012
 *
 * @brief Simple lock-free FIFO.
 * @version 1.1
 * @author Milos Subotic milos.subotic.sm@gmail.com
 *
 * @license MIT
 *
 */

//////////////////////////////////////////////////////////////////////////////

#include "LockFreeFifo.h"

#include <assert.h>

//////////////////////////////////////////////////////////////////////////////

// Compiler barrier, code above and below is not optimized.
#define barrier() do{ __asm__ __volatile__("": : :"memory"); }while(0)

struct _LockFreeFifo {

	volatile size_t _head;
	volatile size_t _tail;

	size_t _capacity;

	LockFreeFifo_elem_t* _buffer;

};

//////////////////////////////////////////////////////////////////////////////

LockFreeFifo LockFreeFifo_create(size_t capacity) {
	LockFreeFifo thiz = malloc(sizeof(struct _LockFreeFifo));

	if(capacity < 2){ // At least one for use and one empty.
		capacity = 2;
	}

	thiz->_buffer = malloc(capacity*sizeof(LockFreeFifo_elem_t));
	assert(thiz->_buffer != NULL);

	thiz->_capacity = capacity;
	thiz->_head = 0;
	thiz->_tail = 0;

	return thiz;
}

void LockFreeFifo_destroy(LockFreeFifo thiz) {
	free(thiz->_buffer);
	free(thiz);
}

size_t LockFreeFifo_capacity(LockFreeFifo thiz) {
	return thiz->_capacity;
}


/**
 * Full space in FIFO. If zero nothing more for get.
 * @return Full space.
 */
size_t LockFreeFifo_size(LockFreeFifo thiz) {
	size_t size = thiz->_head - thiz->_tail;
	// TODO Will compiler optimize this?
	if( size < 0 ){
		size += thiz->_capacity;
	}

	return size;
}

/**
 * Free space in FIFO, If zero no space more for put.
 * @return Free space
 */
size_t LockFreeFifo_free(LockFreeFifo thiz) {
	return thiz->_capacity - 1 - LockFreeFifo_size(thiz);
}

/**
 * Check is FIFO empty, nothing more for get.
 * @return True if FIFO empty.
 */
bool LockFreeFifo_empty(LockFreeFifo thiz) {
	return thiz->_head == thiz->_tail;
}

/**
 * Check is FIFO full, no space more for put.
 * @return True if FIFO full.
 */
bool LockFreeFifo_full(LockFreeFifo thiz) {
	size_t headInc = thiz->_head + 1;

	if(headInc >= thiz->_capacity){ // If head is last and tail is first.
		headInc = 0;
	}

	// TODO Will be thiz->_tail read only once?
	return headInc == thiz->_tail;
}


/**
 * Put data in FIFO.
 * @param thiz Instance of lock-free fifo.
 * @param elem Data element for put.
 * @return False if data element is not put ( FIFO is full ).
 */
bool LockFreeFifo_put(LockFreeFifo thiz, LockFreeFifo_elem_t elem) {
	if(!LockFreeFifo_full(thiz)){
		thiz->_buffer[thiz->_head] = elem;
		size_t newHead = thiz->_head + 1;
		barrier(); // Don't optimize these two together.
		thiz->_head = newHead >= thiz->_capacity ? 0 : newHead;
			return true;
	}else{
		return false;
	}
}


/**
 * Get data from FIFO.
 * @param thiz Instance of lock-free fifo.
 * @param elem Reference for placing read data element.
 * @return False if data element is not got ( FIFO is empty ).
 */
bool LockFreeFifo_get(LockFreeFifo thiz, LockFreeFifo_elem_t* elem) {
	if(!LockFreeFifo_empty(thiz)){
		*elem = thiz->_buffer[thiz->_tail];

		size_t newTail = thiz->_tail + 1;
		barrier(); // Don't optimize these two together.
		thiz->_tail = newTail >= thiz->_capacity ? 0 : newTail;
		return true;
	}else{
		return false;
	}
}

//////////////////////////////////////////////////////////////////////////////
