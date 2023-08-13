//signals.c
//
// Implementations for signals

#include <stdint.h>

#include "signals.h"
#include "memory.h"

SignalQueue kSigs;
SignalQueue* pSigs;

void initSignals() {
	kSigs.first = kSigs.last = 0;
	pSigs = 0;
}

void addProcess(uint32_t IDN) {
	if (pSigs == 0) {
		pSigs = (SignalQueue*)malloc(sizeof(SignalQueue));
		pSigs->IDN = IDN;
		pSigs->first = pSigs->last = 0;
		pSigs->next = 0;
	}
	else {
		SignalQueue* old = pSigs;
		pSigs = (SignalQueue*)malloc(sizeof(SignalQueue));
		pSigs->IDN = IDN;
		pSigs->first = pSigs->last = 0;
		pSigs->next = old;
	}
	return;
}

void removeProcess(uint32_t IDN) {
	SignalQueue* j = 0;
	for (SignalQueue* i = pSigs; i != 0; i = i->next) {
		if (i->IDN == IDN) {
			if (j == 0) pSigs = i->next;
			else j->next = i->next;
			while (getSignal(IDN).type != NONE_SIGNAL);
			free(i);
			return;
		}
		j = i;
	}
}

void _sendSignal(SignalQueue* queueP, Signal sig) {
	SignalQueue queue = *queueP;
	if (queue.first == 0) {
		queue.last = queue.first = (Signal*)malloc(sizeof(Signal));
		*(queue.last) = sig;
		queue.last->next = 0;
	}
	else {
		queue.last->next = (Signal*)malloc(sizeof(Signal));
		queue.last = queue.last->next;
		*(queue.last) = sig;
		queue.last->next = 0;
	}
	*queueP = queue;
	return;
}

Signal _getSignal(SignalQueue* queueP) {
	SignalQueue queue = *queueP;
	if (queue.first == 0) {
		Signal sig;
		sig.type = NONE_SIGNAL;
		return sig;
	}
	Signal* sig = queue.first;
	queue.first = queue.first->next;
	Signal s = *sig;
	free(sig);
	*queueP = queue;
	return s;
}

void sendkSignal(Signal sig) {
	_sendSignal(&kSigs, sig);
	return;
}

Signal getkSignal() {
	return _getSignal(&kSigs);
}

void sendSignal(uint32_t IDN, Signal sig) {
	for (SignalQueue* i = pSigs; i != 0; i = i->next)
		if (i->IDN == IDN) {
			_sendSignal(i, sig);
			return;
		}
}

Signal getSignal(uint32_t IDN) {
	for (SignalQueue* i = pSigs; i != 0; i = i->next)
		if (i->IDN == IDN) return _getSignal(i);
	Signal sig;
	sig.type = NONE_SIGNAL;
	return sig;
}
