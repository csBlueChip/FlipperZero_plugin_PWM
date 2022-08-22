#ifndef  BC_PWMDEMO_H_
#define  BC_PWMDEMO_H_

#include "bc_pwm.h"

//----------------------------------------------------------------------------- ----------------------------------------
// A list of event IDs handled by this plugin
//
typedef
	enum eventID {
		EVID_NONE,
		EVID_UNKNOWN,

		// A full list of events can be found with:  `grep -r --color  "void.*set_.*_callback"  applications/gui/*`
		// ...A free gift to you from the makers of well written code that conforms to a good coding standard
		EVID_KEY,
		EVID_TICK,
	}
eventID_t;

//----------------------------------------------------------------------------- ----------------------------------------
// An item in the event message-queue
//
typedef
	struct eventMsg {
		eventID_t   id;     // --> local
		InputEvent  input;  // --> applications/input/input.h
	}
eventMsg_t;

//----------------------------------------------------------------------------- ----------------------------------------
// State variables for this plugin
// An instance of this is allocated on the heap, and the pointer is passed back to the OS
// Access to this memory is controlled by mutex
//
typedef 
	struct state {
		pwmActor_t*     pwm;

		// We will need a timer to run the animation
	    FuriTimer*      timer;    // the timer
		uint32_t        timerHz;  // system ticks per second
		int             fps;      // events/frames-per-second

		// Editing prams
		int             sel;      // item selected

		// Editable values
		float           fHz;
		float           duty;
		pwmMode_t       mode;
		pwmId_t         hwId;
		uint16_t        prescale;
	}
state_t;

#endif //BC_PWMDEMO_H_
