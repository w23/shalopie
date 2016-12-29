#ifndef SHLP_H__INCLUDED__
#define SHLP_H__INCLUDED__

#define COUNTOF(v) (sizeof(v) / sizeof(*(v)))

typedef int SHLPError;

enum {
	SHLPError_Ok = 0,
	SHLPError_UnexpectedState,
	SHLPError_InvalidArgument,
	SHLPError_Backend
};

typedef enum {
	SHLPMessage_Debug,
	SHLPMessage_Info,
	SHLPMessage_Warning,
	SHLPMessage_Error,
	SHLPMessage_Fatal
} SHLPMessageClass;

void shlpMessage(SHLPMessageClass msg_class, const char *message, ...);

#endif /*ifndef SHLP_H__INCLUDED__*/
