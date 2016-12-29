#ifndef AIO_JACK_H__INCLUDED__
#define AIO_JACK_H__INCLUDED__

#include "shlp.h"
#include "aio.h"

/* Only a single JACK client instance is possible for current implementation */

/* Doesn't start processing yet */
SHLPError aioJackOpen(const char *name, const char *server_name, struct AIOState *aio_state);

/* Callbacks start being called right after this function is entered */
SHLPError aioJackActivate();

/* Callbacks stop being called before this function returns */
SHLPError aioJackDeactivate();

SHLPError aioJackClose();
#endif /*ifndef AIO_JACK_H__INCLUDED__*/
