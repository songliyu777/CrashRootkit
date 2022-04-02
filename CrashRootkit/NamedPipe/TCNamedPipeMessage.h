#pragma once

typedef enum MessageType
{
	MSG_SCRIPT = 0
};

typedef struct _TCMessage
{
	MessageType MsgType;
	SIZE_T size;
	BYTE buff[0x1000];
}TCMessage,*PTCMessage;


