#include "../include/lbq.h"
#include "../../oscl/include/malloc.h"
#include "../../oscl/include/threads.h"

void enqueue(void *self, void *item) {
    //TODO Блокировка очереди с проверкой работы экзекутора и акторов
    LinkedBlockingQueue *this = (LinkedBlockingQueue*) self;
    Node *node = pmalloc(sizeof(Node));

    MutexLock(this->mutex);
    TickType_t taw = xTaskGetTickCount();

    node->item = item;
    node->next = NULL;

    if (this->last == NULL) {
        this->last = node;
        this->head = node;
    } else {
        this->last->next = node;
        this->last = node;
    }

    this->count = (uint16_t) (this->count + 1);

    MutexUnlock(this->mutex);
    if (xTaskGetTickCount() != taw)
       	taskYIELD();
}


void* dequeue(void *self) {
    LinkedBlockingQueue *this = (LinkedBlockingQueue*) self;

    MutexLock(this->mutex);
    TickType_t taw = xTaskGetTickCount();

    if (this->head != NULL) {
        Node *head = this->head;

        void *item = head->item;

        if (this->head->next != NULL) {
            this->head = head->next;
            pfree(head);
        } else {
            pfree(head);
            this->head = NULL;
            this->last = NULL;
        }

        this->count = (uint16_t) (this->count - 1);
        MutexUnlock(this->mutex);
        if (xTaskGetTickCount() != taw)
           	taskYIELD();
        return item;
    } else {
        MutexUnlock(this->mutex);
        if (xTaskGetTickCount() != taw)
           	taskYIELD();
        return NULL;
    }
}

uint16_t size(void *self) {
    LinkedBlockingQueue *this = (LinkedBlockingQueue*) self;
    MutexLock(this->mutex);
    TickType_t taw = xTaskGetTickCount();
    uint16_t size = this->count;
    MutexUnlock(this->mutex);
    if (xTaskGetTickCount() != taw)
       	taskYIELD();
    return size;

}

//Внимание, до вызова функции очередь должна быть полностью очищена
void del_LQB(LinkedBlockingQueue *queue) {
    pfree(queue->mutex);
    pfree(queue);
}

LinkedBlockingQueue* new_LQB(uint16_t capacity) {
    LinkedBlockingQueue* queue = (LinkedBlockingQueue*) pmalloc(sizeof(LinkedBlockingQueue));
    queue->capacity = capacity;
    queue->count = 0;
    queue->head = NULL;
    queue->last = NULL;
    queue->mutex = NewMutex();

    queue->enqueue = enqueue;
    queue->dequeue = dequeue;
    queue->size = size;

    return queue;
}
