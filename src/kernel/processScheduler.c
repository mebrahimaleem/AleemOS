#include <stdint.h>
#include <memory.h>
#include <basicio.h>
#include <kernel.h>
#include <process.h>
#include <taskSwitch.h>
#include <processScheduler.h>

#define LOW_QUANTUM 20
#define NORMAL_QUANTUM 40
#define HIGH_QUANTUM 60
#define BLOCKING_QUANTUM 70

processState* blockingEnqueue;
processState* highEnqueue;
processState* normalEnqueue;
processState* lowEnqueue;

processState* blockingDequeue;
processState* highDequeue;
processState* normalDequeue;
processState* lowDequeue;

processState* _schedulerCurrentProcess;

uint8_t schedulerStatus;
uint8_t processCF; // process control flags
uint32_t schedulerTimestamp;

#pragma GCC push_options
#pragma GCC optimize("O0")
void ISR20_handler(uint32_t opt0) { //from kernel space
	kdata = (volatile KernelData* volatile)(volatile uint32_t)k_KDATA; //Get kernel data
	kdata->systemTime.fraction_ms += kdata->systemTime.fraction_diff;
	kdata->systemTime.whole_ms += kdata->systemTime.whole_diff;

	_schedulerSchedule(opt0);

	return;
}

void farSchedulerEntry(uint32_t cs, uint32_t frame) { //ISR20 from userland (far call)
	kdata = (volatile KernelData* volatile)(volatile uint32_t)k_KDATA; //Get kernel data
	kdata->systemTime.fraction_ms += kdata->systemTime.fraction_diff;
	kdata->systemTime.whole_ms += kdata->systemTime.whole_diff;
	asm volatile ("push eax \n mov al, 0x20 \n out 0x20, al \n pop eax" : : : "memory"); //Tell PIC we are done

	_schedulerSchedule(frame);

	asm volatile ("leave \n retf" : : "m"(cs) : "memory"); //Far return
}
#pragma GCC pop_options

void _schedulerSchedule(uint32_t frame) {
	if (schedulerStatus == 0xff) return;
	
	uint32_t curTime = ((volatile KernelData* volatile)(volatile uint32_t)k_KDATA)->systemTime.whole_ms;
	if (curTime >= schedulerTimestamp) {
		if ((processCF & 1) == 0){
			_schedulerCurrentProcess->cr3 = *(uint32_t* volatile)(frame);
			_schedulerCurrentProcess->edi = *(uint32_t* volatile)(frame+4);
			_schedulerCurrentProcess->esi = *(uint32_t* volatile)(frame+8);
			_schedulerCurrentProcess->ebp = *(uint32_t* volatile)(frame+12);
			_schedulerCurrentProcess->esp = *(uint32_t* volatile)(frame+16);
			_schedulerCurrentProcess->ebx = *(uint32_t* volatile)(frame+20);
			_schedulerCurrentProcess->edx = *(uint32_t* volatile)(frame+24);
			_schedulerCurrentProcess->ecx = *(uint32_t* volatile)(frame+28);
			_schedulerCurrentProcess->eax = *(uint32_t* volatile)(frame+32);
			_schedulerCurrentProcess->eip = *(uint32_t* volatile)(frame+36);
			//skip cs
			_schedulerCurrentProcess->eflags = *(uint32_t* volatile)(frame+44);

			scheduleProcess(_schedulerCurrentProcess);
		}
		else if (blockingDequeue == 0 && lowDequeue == 0 && normalDequeue == 0 && highDequeue == 0) { //No processess remain scheduled
			// Halt operating system
			vgaprint((volatile char* volatile)"All processess have exited! System halt.", 0x04);
			while(1) asm volatile ("cli ; hlt" : : : "memory");
		}

		if (blockingDequeue != 0) { //Check if blocking priority process is waiting
			_schedulerCurrentProcess = blockingDequeue;
			blockingDequeue = blockingDequeue->next;
			schedulerTimestamp = curTime + BLOCKING_QUANTUM;
		}
		else {
			if ((schedulerStatus == 0 && lowDequeue != 0) || (normalDequeue == 0 && highDequeue == 0)) {
				_schedulerCurrentProcess = lowDequeue;
				lowDequeue = lowDequeue->next;
				schedulerTimestamp = curTime + LOW_QUANTUM;
				schedulerStatus = 1;
			}
			else if ((schedulerStatus <= 1 && normalDequeue != 0) || (highDequeue == 0)) {
				_schedulerCurrentProcess = normalDequeue;
				normalDequeue = normalDequeue->next;
				schedulerTimestamp = curTime + NORMAL_QUANTUM;
				schedulerStatus = 2;
			}
			else {
				_schedulerCurrentProcess = highDequeue;
				highDequeue = highDequeue->next;
				schedulerTimestamp = curTime + HIGH_QUANTUM;
				schedulerStatus = 0;
			}
		}

		processCF = 0;
		
		if (_schedulerCurrentProcess->cr3 == 0xc000) // Check if process is from kernel
			kernelReentry((uint32_t)_schedulerCurrentProcess);

		startProcess(_schedulerCurrentProcess);
	}

	return;
}

void scheduleProcess(processState* state) {
	if ((state->priority & 3)== 0) { //Low
		if (lowEnqueue == 0 || lowDequeue == 0 ){ 
			lowEnqueue = lowDequeue = state;
			lowEnqueue->next = 0;
		}
		else {
			lowEnqueue->next = state;
			lowEnqueue = state;
		}
	}
	else if ((state->priority&3) == 1) { //Normal
		if (normalEnqueue == 0 || normalDequeue == 0){
			normalEnqueue = normalDequeue = state;
			normalEnqueue->next = 0;
		}
		else {
			normalEnqueue->next = state;
			normalEnqueue = state;
		}
	}
	else if ((state->priority&3) == 2) { //High
		if (highEnqueue == 0 || highDequeue == 0){
			highEnqueue = highDequeue = state;
			highEnqueue->next = 0;
		}
		else {
			highEnqueue->next = state;
			highEnqueue = state;
		}
	}
	else { //Blocking
		if (blockingEnqueue == 0 || blockingDequeue == 0) blockingEnqueue = blockingDequeue = state;
		else {
			blockingEnqueue->next = state;
			blockingEnqueue = state;
		}
	}
}

void unscheduleCurrentProcess() {
	processCF |= 1;

	// suspend current process until next switch
	while(1) asm volatile ("sti ; hlt" : : : "memory");
}

void initScheduler(void) {
	blockingEnqueue = blockingDequeue = highEnqueue = highDequeue = normalEnqueue = normalDequeue = lowEnqueue = lowDequeue = 0;
	_schedulerCurrentProcess = (processState* volatile)malloc(sizeof(processState));
	_schedulerCurrentProcess->PID = 0;
	_schedulerCurrentProcess->priority = 2;
	
	schedulerTimestamp = 0;
	processCF = 0;
}
