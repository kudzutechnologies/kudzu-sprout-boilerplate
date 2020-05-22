#ifndef DOTCONFIG_DECLARATIONS_H
#define DOTCONFIG_DECLARATIONS_H
#include <string.h>

/**
 * Callback function used by the server to render configuration
 * chunks directly to the output without the need for intermediate
 * buffers.
 */
typedef void (*writeChunkCallback)(const char*, size_t, void*);

/**
 * Button widget callback function, that will be triggered every time the
 * value is included in the response (eg. when a button is pressed)
 */
typedef void (*buttonPressCallback)(bool pressed, void * userdata);

/**
 * Generator callback, that can be used to produce a custom JSON value
 * for the specific data widget.
 */
typedef void (*generatorCallback)(writeChunkCallback cb, void * chunkData, void * userdata);

#endif
