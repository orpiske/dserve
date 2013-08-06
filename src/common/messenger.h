#ifndef MESSENGER_H_
#define MESSENGER_H_

#include <stdlib.h>

typedef enum message_level_t_ {
	TRACE,
	DEBUG,
	INFO,
	WARNING,
	ERROR,
	FATAL,
} message_level_t;

typedef void(*messenger)(message_level_t level, const char *message, ...);

void set_messenger(messenger new_msg);
messenger get_messenger(void);

//MESSENGER_H
#endif
