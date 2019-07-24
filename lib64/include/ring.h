/*
 * Copyright (c) 2019 Jie Zheng
 */
#ifndef _BYTE_RING_H
#define _BYTE_RING_H
/*
 * the ring is to implement byte stream queue with a static byte array.
 * it's not smp safe.
 */
#include <lib64/include/type.h>

struct ring {
    // the ring_size is the capacity of the ring plus 1
    int32_t ring_size;
    int32_t front;
    int32_t rear;
    uint8_t buffer[0];
}__attribute__((packed));

#define ring_reset(_ring) {\
    (_ring)->front = 0; \
    (_ring)->rear = 0; \
}

#define ring_empty(_ring) ({\
    (_ring)->front == (_ring)->rear; \
})

#define ring_full(_ring) ({\
    (((_ring)->rear + 1) % (_ring)->ring_size) == (_ring)->front; \
})

__attribute__((always_inline)) static inline int32_t
ring_length(struct ring * ring)
{
    return (ring->rear + ring->ring_size - ring->front) % ring->ring_size;
}

__attribute__((always_inline)) static inline int32_t
write_ring(struct ring * ring, void * buffer, int32_t size)
{
    int32_t left = size;
    int32_t result = 0;
    uint8_t * ptr = (uint8_t *)buffer;
    while (left > 0) {
        if (ring_full(ring))
            break;
        ring->buffer[ring->rear] = ptr[result];
        ring->rear = (ring->rear + 1) % ring->ring_size;
        result += 1;
        left -= 1;
    }
    return result;
}

__attribute__((always_inline)) static inline int32_t
read_ring(struct ring * ring, void * buffer, int32_t size)
{
    uint8_t * ptr = (uint8_t *)buffer;
    int32_t left = size;
    int32_t result = 0;

    while (left > 0) {
        if (ring_empty(ring))
            break;
        ptr[result] = ring->buffer[ring->front];
        ring->front = (ring->front + 1) % ring->ring_size;
        result += 1;
        left -= 1;
    }
    return result;
}
// On sucess, it returns 1
__attribute__((always_inline)) static inline int32_t
ring_enqueue(struct ring * ring, uint8_t value)
{
    return write_ring(ring, &value, 1);
}

__attribute__((always_inline)) static inline int32_t
ring_dequeue(struct ring * ring, uint8_t * value)
{
    return read_ring(ring, value, 1);
}

__attribute__((always_inline)) static inline uint8_t
ring_peek(struct ring * _ring)
{
    return _ring->buffer[_ring->front];
}

#endif
