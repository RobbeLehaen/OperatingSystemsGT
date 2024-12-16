#include <stdlib.h>
#include <pthread.h>
#include "sbuffer.h"
#include "config.h"
#include "connmgr.h"
#include <inttypes.h>
#include <stdio.h>

struct sbuffer_node {
    sensor_data_t data;
    struct sbuffer_node *next;
};

struct sbuffer {
    struct sbuffer_node *head;
    struct sbuffer_node *tail;
    pthread_mutex_t buffer_lock;
    pthread_cond_t buffer_not_empty;
    size_t size;
};

int sbuffer_init(sbuffer_t **buffer) {
    if (buffer == NULL) return SBUFFER_FAILURE;

    *buffer = malloc(sizeof(sbuffer_t));
    if (*buffer == NULL) return SBUFFER_FAILURE;

    (*buffer)->head = NULL;
    (*buffer)->tail = NULL;
    (*buffer)->size = 0;

    if (pthread_mutex_init(&((*buffer)->buffer_lock), NULL) != 0) {
        free(*buffer);
        return SBUFFER_FAILURE;
    }

    if (pthread_cond_init(&((*buffer)->buffer_not_empty), NULL) != 0) {
        pthread_mutex_destroy(&((*buffer)->buffer_lock));
        free(*buffer);
        return SBUFFER_FAILURE;
    }

    return SBUFFER_SUCCESS;
}


// Free the shared buffer
int sbuffer_free(sbuffer_t **buffer) {
    if (buffer == NULL || *buffer == NULL) return SBUFFER_FAILURE;

    sbuffer_t *buf = *buffer;

    pthread_mutex_lock(&(buf->buffer_lock));

    struct sbuffer_node *current = buf->head;
    while (current != NULL) {
        struct sbuffer_node *to_free = current;
        current = current->next;
        free(to_free);
    }

    buf->head = NULL;
    buf->tail = NULL;
    buf->size = 0;

    pthread_cond_broadcast(&(buf->buffer_not_empty));  // Wake up any waiting threads
    pthread_mutex_unlock(&(buf->buffer_lock));

    pthread_mutex_destroy(&(buf->buffer_lock));
    pthread_cond_destroy(&(buf->buffer_not_empty));

    free(buf);
    *buffer = NULL;

    return SBUFFER_SUCCESS;
}

// Insert data into the shared buffer
int sbuffer_insert(sbuffer_t *buffer, sensor_data_t *data) {
    if (buffer == NULL || data == NULL) return SBUFFER_FAILURE;

    pthread_mutex_lock(&(buffer->buffer_lock));

    struct sbuffer_node *new_node = malloc(sizeof(struct sbuffer_node));
    if (new_node == NULL) {
        pthread_mutex_unlock(&(buffer->buffer_lock));
        return SBUFFER_FAILURE;
    }

    new_node->data = *data;
    new_node->data.processed = 0;
    new_node->next = NULL;

    if (buffer->tail == NULL) {
        buffer->head = new_node;
        buffer->tail = new_node;
    } else {
        buffer->tail->next = new_node;
        buffer->tail = new_node;
    }

    buffer->size++;
    pthread_cond_signal(&(buffer->buffer_not_empty));

    pthread_mutex_unlock(&(buffer->buffer_lock));

    return SBUFFER_SUCCESS;
}


// Remove data from the shared buffer
int sbuffer_remove(sbuffer_t *buffer, sensor_data_t **data) {
    if (buffer == NULL || data == NULL) return SBUFFER_FAILURE;

    pthread_mutex_lock(&(buffer->buffer_lock));

    while (buffer->size == 0) {
        if (pthread_cond_wait(&(buffer->buffer_not_empty), &(buffer->buffer_lock)) != 0) {
            pthread_mutex_unlock(&(buffer->buffer_lock));
            return SBUFFER_FAILURE;
        }
    }

    struct sbuffer_node *node_to_remove = buffer->head;

    // Allocate memory for the data being removed
    *data = malloc(sizeof(sensor_data_t));
    if (*data == NULL) {
        pthread_mutex_unlock(&(buffer->buffer_lock));
        return SBUFFER_FAILURE;
    }

    // Copy data from the node
    **data = node_to_remove->data;

    // Update the head of the buffer
    buffer->head = node_to_remove->next;
    if (buffer->head == NULL) {
        buffer->tail = NULL;
    }

    free(node_to_remove); // Free the buffer node itself
    buffer->size--;

    pthread_mutex_unlock(&(buffer->buffer_lock));

    return SBUFFER_SUCCESS;
}

int sbuffer_peek(sbuffer_t *buffer, sensor_data_t **data) {
    if (buffer == NULL || data == NULL) {
        return SBUFFER_FAILURE;
    }

    pthread_mutex_lock(&(buffer->buffer_lock));

    if (buffer->size == 0) {
        pthread_mutex_unlock(&(buffer->buffer_lock));
        return SBUFFER_EMPTY;
    }

    *data = &(buffer->head->data);

    pthread_mutex_unlock(&(buffer->buffer_lock));
    return SBUFFER_SUCCESS;
}