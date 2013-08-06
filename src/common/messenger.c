#include "messenger.h"

static messenger msg = NULL;

void set_messenger(messenger new_msg) {
	msg = new_msg;
}


messenger get_messenger(void) {
	return msg;
}
