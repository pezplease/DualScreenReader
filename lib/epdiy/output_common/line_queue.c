#include <string.h>
#include <esp_attr.h>

#include "line_queue.h"

uint8_t* IRAM_ATTR lq_current(LineQueue_t* queue) {
    int current = atomic_load_explicit(&queue->current, memory_order_acquire);
    int last = atomic_load_explicit(&queue->last, memory_order_acquire);

    if ((current  + 1) % queue->size == last) {
        return NULL;
    }
    return &queue->buf[current * queue->element_size];
}

void IRAM_ATTR lq_commit(LineQueue_t* queue) {

    int current = atomic_load_explicit(&queue->current, memory_order_acquire);
    if (current == queue->size - 1) {
        queue->current = 0;
    } else {
        atomic_fetch_add(&queue->current, 1);
    }
}

int IRAM_ATTR lq_read(LineQueue_t* queue, uint8_t* dst) {
    int current = atomic_load_explicit(&queue->current, memory_order_acquire);
    int last = atomic_load_explicit(&queue->last, memory_order_acquire);

    if (current == last) {
        return -1;
    }

    memcpy(dst, &queue->buf[last * queue->element_size], queue->element_size);

    if (last == queue->size - 1) {
        queue->last = 0;
    } else {
        atomic_fetch_add(&queue->last, 1);
    }
    return 0;
}


void IRAM_ATTR lq_reset(LineQueue_t* queue) {
    queue->current = 0;
    queue->last = 0;
}
