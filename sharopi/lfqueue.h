#ifndef LFQUEUE_H__INCLUDED
#define LFQUEUE_H__INCLUDED

/* Ring-buffer-based allocation-less bounded queue */
/* TODO: multi-producer single-consumer lock-free (needed for unsync MIDI and evdev event sources) */

#ifndef LFQUEUE_SIZE_TYPE
#define LFQUEUE_SIZE_TYPE unsigned int
#endif

/* TODO: suffix to enable differently-typed queues */

typedef LFQUEUE_SIZE_TYPE LFQSize;

struct LFQueue {
	LFQSize read_pos_, write_pos_;
	unsigned char *buffer_;
	LFQSize buffer_size_;
};

struct LFQMemBlock {
	LFQSize size;
	const void *data;
};

void lfqInit(struct LFQueue *queue, LFQSize buffer_size, void *buffer);

/* returns 0 on success, or number of missing free bytes on error */
LFQSize lfqWrite(struct LFQueue *queue, LFQSize size, const void *data);

/* Write multiple sections atomically */
/* returns 0 on success, or number of missing free bytes on error */
LFQSize lfqWriteMany(struct LFQueue *queue, LFQSize blocks_count, const struct LFQMemBlock *blocks);

/* return number of bytes actually read */
LFQSize lfqRead(struct LFQueue *queue, LFQSize size, void *data);
#endif /*ifndef LFQUEUE_H__INCLUDED*/

#ifdef LFQ_IMPLEMENT
#ifndef LFQ_IMPLEMENTED_HERE
#define LFQ_IMPLEMENTED_HERE

void lfqInit(struct LFQueue *queue, LFQSize buffer_size, void *buffer) {
	queue->buffer_ = buffer;
	queue->buffer_size_ = buffer_size;
	queue->read_pos_ = queue->write_pos_ = 0;
}

LFQSize lfqWrite(struct LFQueue *queue, LFQSize size, const void *data) {
	LFQSize read_pos = queue->read_pos_;
	LFQSize alloc_begin = queue->write_pos_;

	if (queue->buffer_size_ <= size) return size - queue->buffer_size_;

	if (read_pos > alloc_begin) {
		LFQSize free = read_pos - alloc_begin - 1; /* 1 is needed to avoid r == w on full queue, (r == w) means empty */
		if (free < size) return size - free;

		memcpy(queue->buffer_ + alloc_begin, data, size);
	} else { /* read_pos <= alloc_begin; wraparound */
		LFQSize free = read_pos + queue->buffer_size_ - alloc_begin - 1;
		LFQSize head = queue->buffer_size_ - alloc_begin;
		if (free < size) return size - free;

		if (head > size) {
			memcpy(queue->buffer_ + alloc_begin, data, size);
		} else {
			memcpy(queue->buffer_ + alloc_begin, data, head);
			memcpy(queue->buffer_, (unsigned char*)data + head, size - head);
		}
	} /* wraparound case */

	queue->write_pos_ = (alloc_begin + size) % queue->buffer_size_;
	return 0;
}

/* TODO
LFQSize lfqWriteMany(struct LFQueue *queue, LFQSize blocks_count, const struct LFQMemBlock *blocks) {
}
*/

LFQSize lfqRead(struct LFQueue *queue, LFQSize size, void *data) {
	LFQSize read_pos = queue->read_pos_;
	LFQSize write_pos = queue->write_pos_;

	if (read_pos <= write_pos) {
		LFQSize have = write_pos - read_pos;
		LFQSize have_read = have > size ? size : have;
		if (data) memcpy(data, queue->buffer_ + read_pos, have_read);
		queue->read_pos_ += have_read;
		return have_read;
	} else /* read_pos > write_pos; wraparound */ {
		LFQSize head = queue->buffer_size_ - read_pos;
		if (head > size) {
			if (data) memcpy(data, queue->buffer_ + read_pos, size);
			queue->read_pos_ += size;
			return size;
		} else {
			LFQSize wrap = head - size;
			wrap = wrap > write_pos ? write_pos : wrap;

			if (data) {
				memcpy(data, queue->buffer_ + read_pos, head);
				memcpy((unsigned char*)data + head, queue->buffer_, wrap);
			}
			queue->read_pos_ = wrap;
			return size;
		}
	} /* wraparound case */
}

#endif /*ifndef LFQ_IMPLEMENTED_HERE*/
#endif /*ifdef LFQ_IMPLEMENT*/

