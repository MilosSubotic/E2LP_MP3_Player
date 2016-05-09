/**
 * @file LockFreeFifo.h
 * @date Dec 14, 2012
 *
 * @brief Simple lock-free FIFO.
 * @version 1.1
 * @author Milos Subotic milos.subotic.sm@gmail.com
 *
 * @license MIT
 *
 */

#ifndef LOCKFREEFIFO_H_
#define LOCKFREEFIFO_H_

//////////////////////////////////////////////////////////////////////////////

// For size_t definition.
#include <stdlib.h>

// For bool.
#include <stdbool.h>

//////////////////////////////////////////////////////////////////////////////

typedef void* LockFreeFifo_elem_t;

typedef struct _LockFreeFifo* LockFreeFifo;

//////////////////////////////////////////////////////////////////////////////

LockFreeFifo LockFreeFifo_create(size_t capacity);

void LockFreeFifo_destroy(LockFreeFifo thiz);

size_t LockFreeFifo_capacity(LockFreeFifo thiz);


/**
 * Full space in FIFO. If zero nothing more for get.
 * @param thiz Instance of lock-free fifo.
 * @return Full space.
 */
size_t LockFreeFifo_size(LockFreeFifo thiz);

/**
 * Free space in FIFO, If zero no space more for put.
 * @param thiz Instance of lock-free fifo.
 * @return Free space
 */
size_t LockFreeFifo_free(LockFreeFifo thiz);

/**
 * Check is FIFO empty, nothing more for get.
 * @param thiz Instance of lock-free fifo.
 * @return True if FIFO empty.
 */
bool LockFreeFifo_empty(LockFreeFifo thiz);

/**
 * Check is FIFO full, no space more for put.
 * @param thiz Instance of lock-free fifo.
 * @return True if FIFO full.
 */
bool LockFreeFifo_full(LockFreeFifo thiz);

/**
 * Put data in FIFO.
 * @param thiz Instance of lock-free fifo.
 * @param elem Data element for put.
 * @return False if data element is not put ( FIFO is full ).
 */
bool LockFreeFifo_put(LockFreeFifo thiz, LockFreeFifo_elem_t elem);

/**
 * Get data from FIFO.
 * @param thiz Instance of lock-free fifo.
 * @param elem Reference for placing read data element.
 * @return False if data element is not got ( FIFO is empty ).
 */
bool LockFreeFifo_get(LockFreeFifo thiz, LockFreeFifo_elem_t* elem);

//////////////////////////////////////////////////////////////////////////////

#endif /* LOCKFREEFIFO_H_ */
