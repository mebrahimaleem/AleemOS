/*
MIT License

Copyright 2022-2024 Ebrahim Aleem

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
*/

extern void ISR20_handler(uint32_t opt0);

extern void scheduleProcess(processState* state);

extern void initScheduler(void);

extern void restartProcess(uint32_t curTime);

extern uint8_t killProcess(uint32_t PID);

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
