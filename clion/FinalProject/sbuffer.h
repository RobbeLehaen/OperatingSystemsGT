/**
 * \author {AUTHOR}
 */

#ifndef _SBUFFER_H_
#define _SBUFFER_H_

#include "config.h"

#define SBUFFER_FAILURE -1
#define SBUFFER_SUCCESS 0
#define SBUFFER_NO_DATA 1
#define SBUFFER_EMPTY 2
#define SBUFFER_DUPLICATE 3

typedef struct sbuffer sbuffer_t;

int sbuffer_init(sbuffer_t **buffer);

int sbuffer_free(sbuffer_t **buffer);

int sbuffer_remove(sbuffer_t *buffer, sensor_data_t **data);

int sbuffer_insert(sbuffer_t *buffer, sensor_data_t *data);

int sbuffer_peek(sbuffer_t *buffer, sensor_data_t **data);

#endif  //_SBUFFER_H_
