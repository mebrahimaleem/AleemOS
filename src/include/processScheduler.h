extern void ISR20_handler(uint32_t opt0);

extern void scheduleProcess(processState* state);

extern void unscheduleCurrentProcess(void);

extern void initScheduler(void);

extern void farSchedulerEntry(uint32_t cs, uint32_t frame);

extern void _schedulerSchedule(uint32_t frame);

extern uint8_t schedulerStatus;

extern processState* _schedulerCurrentProcess;

extern uint32_t schedulerTimestamp;

extern processState* blockingEnqueue;
extern processState* highEnqueue;
extern processState* normalEnqueue;
extern processState* lowEnqueue;

extern processState* blockingDequeue;
extern processState* highDequeue;
extern processState* normalDequeue;
extern processState* lowDequeue;
