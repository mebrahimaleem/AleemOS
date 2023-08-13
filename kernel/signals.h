//signals.h
//
// Declarations for signals

typedef enum{
	USB_SIGNAL,
	NONE_SIGNAL
} SignalType;

typedef struct Signal{
	SignalType type;
	uint8_t* options;
	struct Signal* next;
} Signal;

typedef struct SignalQueue{
	Signal* first;
	Signal* last;
	struct SignalQueue* next;
	uint32_t IDN;
} SignalQueue;

extern void initSignals(void);
extern void addProcess(uint32_t IDN);
extern void removeProcess(uint32_t IDN);

extern void sendSignal(uint32_t IDN, Signal sig);
extern Signal getSignal(uint32_t IDN);

extern void sendkSignal(Signal sig);
extern Signal getkSignal(void);

extern void _sendSignal(SignalQueue* queueP, Signal sig);
extern Signal _getSignal(SignalQueue* queueP);
