//signals.h
//
// Declarations for signals

/*
	Enumerator for types of signals
*/
typedef enum{
	USB_SIGNAL, // Signal from USB device
	NONE_SIGNAL // Absent signal (No signals on signal queue)
} SignalType;

/*
	Structure to hold information on all signal for a process
*/
typedef struct Signal{
	SignalType type; //Signal type
	uint8_t* options; //Options for the signal
	struct Signal* next; // Pointer to next signal
} Signal;

/*
	Queue for holding all signals for a process (FIFO)
*/
typedef struct SignalQueue{
	Signal* first; // First (oldest) signal
	Signal* last; // Last (newest) signal
	struct SignalQueue* next; // Pointer for signalQueue for the next process
	uint32_t PID;
} SignalQueue;

/*
	Sets up structures for signal management
*/
extern void initSignals(void);

/*
	Registers a process for receiving signals
*/
extern void addProcess(uint32_t PID);

/* 
	Unregisters a process from receiving signals
*/
extern void removeProcess(uint32_t PID);

/*
	Send a signal to a process
	sig: Signal to send
*/
extern void sendSignal(uint32_t PID, Signal sig);

/*
	Get a processes oldest signal

	Returns the signal
*/
extern Signal getSignal(uint32_t PID);

/*
	Send a signal to the kernel/drivers
	sig: Signal to send
*/
extern void sendkSignal(Signal sig);

/*
	Gets the oldest signal for the kernel

	return the oldest Signal
*/
extern Signal getkSignal(void);

/*
	Sends a signal to a SignalQueue

	queueP: Pointer to the signal queue
	sig: Signal to send
*/
extern void _sendSignal(SignalQueue* queueP, Signal sig);

/*
	Get the oldest signal from a signal queue
	queueP: Pointer to the signal queue

	returns the oldest signal
*/
extern Signal _getSignal(SignalQueue* queueP);
