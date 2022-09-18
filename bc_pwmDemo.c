//----------------------------------------------------------------------------- ----------------------------------------
// System libs
#include  <stdlib.h>  // malloc
#include  <stdint.h>  // uint32_t
#include  <stdarg.h>  // __VA_ARGS__

// FlipperZero libs
#include  <furi.h>               // Core API
//#include  <furi_hal_infrared.h>  // NOT included by furi_hal.h !?
#include  <gui/gui.h>            // GUI (screen/keyboard) API
#include  <gui/elements.h>
#include  <input/input.h>        // GUI Input extensions
//#include  <furi_hal_gpio.h>      // GPIO
//#include <furi_hal_resources.h>

//#include <stm32wbxx_ll_tim.h>
//#include <stm32wbxx_ll_comp.h>

// Error Messages
#define ERR_C_
#include "err.h"

// The FlipperZero Settings->System menu allows you to set the logging level at RUN-time
// LOG_LEVEL lets you limit it at COMPILE-time
//    1.  None
//    2.  Errors      : ERROR -> FURI_LOG_E
//    3.  Warnings    : WARN  -> FURI_LOG_W
//    4.  Information : INFO  -> FURI_LOG_I
//    5.  Debug       : DEBUG -> FURI_LOG_D
//    6.  Trace       : TRACE -> FURI_LOG_T
// Also provides ENTER and LEAVE -> TRACE
#define  LOG_LEVEL  6
#include  "bc_logging.h"

// Local headers
#include  "bc_pwmDemo.h"  // Various enums and struct declarations
//#include  "err.h"         // Error numbers & messages

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//   OOOOO    // SSSSS       CCCCC  AAA  L     L     BBBB   AAA   CCCC K   K  SSSSS
//   O   O   /// S           C     A   A L     L     B   B A   A C     K  K   S
//   O   O  ///  SSSSS       C     AAAAA L     L     BBBB  AAAAA C     KKK    SSSSS
//   O   O ///       S       C     A   A L     L     B   B A   A C     K  K       S
//   OOOOO //    SSSSS       CCCCC A   A LLLLL LLLLL BBBB  A   A  CCCC K   K  SSSSS
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//+============================================================================ ========================================
// OS Callback : Timer tick
// We register this function to be called when the OS signals a tick event
//
static
void  cbTimer (FuriMessageQueue* queue)
{
	ENTER;
	furi_assert(queue);

	eventMsg_t  message = {.id = EVID_TICK};
	furi_message_queue_put(queue, &message, 0);

	LEAVE;
	return;
}

//+============================================================================ ========================================
// OS Callback : Keypress
// We register this function to be called when the OS detects a keypress
//
static
void  cbInput (InputEvent* event,  FuriMessageQueue* queue)
{
	ENTER;
	furi_assert(queue);
	furi_assert(event);

	// Put an "input" event message on the message queue
	eventMsg_t  message = {.id = EVID_KEY,  .input = *event};
	furi_message_queue_put(queue, &message, FuriWaitForever);

	LEAVE;
	return;
}

//+============================================================================ ========================================
// OS Callback : Draw request
// We register this function to be called when the OS requests that the screen is redrawn
//
// We actually instruct the OS to perform this request, after we update the interface
// I guess it's possible that this instruction may able be issued by other threads !?
//
#define ALIGN_TL  AlignLeft, AlignTop
#define ALIGN_TR  AlignRight, AlignTop
#define LINE(fmt, ...)  do { \
	snprintf(s, sizeof(s), fmt __VA_OPT__(,) __VA_ARGS__); \
	canvas_set_color(canvas, (y == state->sel) ? 0 : 1); \
	canvas_draw_str_aligned(canvas, 2, y *(fontH +2), ALIGN_TL, s); \
	y++; \
}while(0)

static
void  cbDraw (Canvas* const canvas,  void* ctx)
{
	ENTER;
	furi_assert(canvas);
	furi_assert(ctx);

	char      s[32];
	state_t*  state;
	int       fontH;
	int       y;

	if ( !(state = (state_t*)acquire_mutex((ValueMutex*)ctx, 25)) )  return ;

	// Border around the edge of the screen
	// top-left is {0,0}, [standard] screen is 128x64 {WxH}
//	canvas_draw_frame(canvas, 0, 0, canvas_width(canvas), canvas_height(canvas));

	canvas_set_font(canvas, FontPrimary);
	fontH = canvas_current_font_height(canvas);

	elements_slightly_rounded_box(canvas, 0, (state->sel *(fontH +2)) -2, 128, fontH+2);

	y = 0;
    LINE("PWM Test [CSBC]");
	LINE("Freq Hz  : %.2f", (double)(state->fHz));
	LINE("Duty %%   : %.2f", (double)(state->duty *100));
	LINE("Mode     : %s", 
		(state->mode == PWM_MODE_DUTY) ? "Duty" : 
		(state->mode == PWM_MODE_VOLUME) ? "Volume" : 
		"Brightness" );
 	LINE("Output   : %s", pwmHw[state->hwId].name);

	canvas_set_color(canvas, 1);
	canvas_draw_line(canvas, 0,fontH-1, 128,fontH-1);
	canvas_draw_str_aligned(canvas, 128, 1, ALIGN_TR, (state->pwm->run) ? "ON" : "OFF");

	release_mutex((ValueMutex*)ctx, state);

	LEAVE;
	return;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//   EEEEE V   V EEEEE N   N TTTTT       H   H  AAA  N   N DDDD  L     EEEEE RRRR  SSSSS
//   E     V   V E     NN  N   T         H   H A   A NN  N  D  D L     E     R   R S
//   EEE    V V  EEE   N N N   T         HHHHH AAAAA N N N  D  D L     EEE   RRRR  SSSSS
//   E      V V  E     N  NN   T         H   H A   A N  NN  D  D L     E     R R       S
//   EEEE    V   EEEEE N   N   T         H   H A   A N   N DDDD  LLLLL EEEEE R  R  SSSSS
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//+============================================================================ ========================================
// Event Handler : Tick
//
static
void  evTick (state_t* state)
{
	ENTER;
	furi_assert(state);

	(void)0;

	LEAVE;
	return;
}

//+============================================================================ ========================================
// Handle a key press event
//
static inline
bool  evKey (eventMsg_t* msg,  state_t* state, Gui* gui)
{
	ENTER;
	furi_assert(msg);
	furi_assert(state);
	furi_assert(gui);

	bool  run = true;  // assume keep running

	switch (msg->input.type) {
		case InputTypeShort:   // Short   - after InputTypeRelease within INPUT_LONG_PRESS interval
			break;

		case InputTypeLong:    // Long    - after INPUT_LONG_PRESS interval, asynch to InputTypeRelease
			break;

		case InputTypePress:   // Press   - after debounce
			switch (msg->input.key) {
				case InputKeyOk:
					pwmInit(state->pwm, state->hwId, state->prescale, state->mode);
					pwmSet(state->pwm, state->fHz, state->duty, true);  // true = start running now
//					pwmRun(state->pwm);
					break;
				case InputKeyUp:
					if (state->sel > 1)  state->sel-- ;
					break;
				case InputKeyDown:
					if (state->sel < 4)  state->sel++ ;
					break;
				case InputKeyLeft:
				case InputKeyRight:
				case InputKeyBack:
					break;

				// Unknown key
				default:
					if (msg->input.type == InputTypePress)
						WARN("%s : Unknown key [%d]", __func__, msg->input.key);
					break;
			}
			// Now treat the keystroke like a repeat
			__attribute__ ((fallthrough));

		case InputTypeRepeat:  // Repeat  - with INPUT_REPEATE_PRESS period after InputTypeLong event
			switch (msg->input.key) {
				case InputKeyLeft:
					switch (state->sel) {
						case 1:
							if (state->fHz)   state->fHz  -= 20 ;
							break;
						case 2:
							if (state->duty)  state->duty -= 0.02 ;
							break;
						case 3:
							if (state->mode)  state->mode-- ;
							break;
						case 4:
							if (state->hwId)  state->hwId-- ;
							break;
					}
					break;
				case InputKeyRight:
					switch (state->sel) {
						case 1:
							if (state->fHz  < 20000          )  state->fHz  += 20 ;
							break;
						case 2:
							if (state->duty < 1.0            )  state->duty += 0.02 ;
							break;
						case 3:
							if (state->mode < PWM_MODE_CNT -1)  state->mode++ ;
							break;
						case 4:
							if (state->hwId < PWM_ID_CNT -1  )  state->hwId++ ;
							break;
					}
					break;
				default:
					break;
			}
			break;

		case InputTypeRelease: // Release - after debounce
			switch (msg->input.key) {
				case InputKeyOk:
					(void)pwmStop(state->pwm);
					break;
				case InputKeyBack:
					run = false ;  // Signal the plugin to exit
					break;
				default:
					break;
			}
			break;

		default:
			WARN("%s : Unknown event [%d]", __func__, msg->input.type);
			break;
	}

	LEAVE;
	return run;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//   SSSSS TTTTT  AAA  TTTTT EEEEE       V   V  AAA  RRRR  IIIII  AAA  BBBB  L     EEEEE SSSSS
//   S       T   A   A   T   E           V   V A   A R   R   I   A   A B   B L     E     S
//   SSSSS   T   AAAAA   T   EEE          V V  AAAAA RRRR    I   AAAAA BBBB  L     EEE   SSSSS
//       S   T   A   A   T   E            V V  A   A R R     I   A   A B   B L     E         S
//   SSSSS   T   A   A   T   EEEEE         V   A   A R  R  IIIII A   A BBBB  LLLLL EEEEE SSSSS
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//+============================================================================ ========================================
// Initialise plugin state variables
//
static inline
bool  stateInit (state_t* const state,  const Gui* gui)
{
	ENTER;
	furi_assert(state);
	furi_assert(gui);

	bool rv = true;  // assume success

	state->timer    = NULL;
	state->timerHz  = furi_kernel_get_tick_frequency();
	state->fps      = 12;

	state->sel      = 1;

	state->fHz      = 1000;
	state->duty     = 0.5;
	state->mode     = PWM_MODE_DUTY;
	state->hwId     = PWM_ID_IR;
	state->prescale = 500;  //! Where does 500 come from? [other than "the speaker demo code"]

	LEAVE;
	return rv;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//   MM MM  AAA  IIIII N   N
//   M M M A   A   I   NN  N
//   M M M AAAAA   I   N N N
//   M   M A   A   I   N  NN
//   M   M A   A IIIII N   N
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//+============================================================================ ========================================
// Plugin entry point
//
int32_t  bc_pwm_demo (void)
{
	ENTER;

	// ===== Variables =====
	err_t              error   = 0;    // assume success

	Gui*               gui     = NULL;
	ViewPort*          vpp     = NULL;

	state_t*           state   = NULL;
	ValueMutex         mutex   = {0};

	FuriMessageQueue*  queue   = NULL;
	const uint32_t     queueSz = 8;    // maximum messages in queue
	eventMsg_t         msg     = {0};

	bool               run     = 1;

	INFO("BEGIN");

	// ===== Message queue =====
	// 1. Create a message queue (for up to 8 (keyboard) event messages)
	if ( !(queue = furi_message_queue_alloc(queueSz, sizeof(msg))) ) {
		ERROR(pwm_errs[(error = ERR_MALLOC_QUEUE)]);
		goto bail;
	}

	// ===== Create GUI Interface =====
	// 2. Create a GUI interface
	if ( !(gui = furi_record_open("gui")) ) {
		ERROR(pwm_errs[(error = ERR_NO_GUI)]);
		goto bail;
	}

	// ===== Plugin state variables =====
	// 3. Allocate space on the heap for the plugin state variables
	if ( !(state = malloc(sizeof(*state))) ) {
		ERROR(pwm_errs[(error = ERR_MALLOC_STATE)]);
		goto bail;
	}
	// 4. Initialise the plugin state variables
	if (!stateInit(state, gui)) {
		// error message(s) is/are output by stateInit()
		error = 15;
		goto bail;
	}
	// 5. Create a mutex for (reading/writing) the plugin state variables
	if (!init_mutex(&mutex, state, sizeof(state))) {
		ERROR(pwm_errs[(error = ERR_NO_MUTEX)]);
		goto bail;
	}

	// ===== Viewport =====
	// 6. Allocate space on the heap for the viewport
	if ( !(vpp = view_port_alloc()) ) {
		ERROR(pwm_errs[(error = ERR_MALLOC_VIEW)]);
		goto bail;
	}
	// 7a. Register a callback for input events
	view_port_input_callback_set(vpp, cbInput, queue);
	// 7b. Register a callback for draw events
	view_port_draw_callback_set(vpp, cbDraw, &mutex);

	// ===== Start GUI Interface =====
	// 8. Attach the viewport to the GUI
	gui_add_view_port(gui, vpp, GuiLayerFullscreen);

	// ===== Timer =====
	// 9. Allocate a timer
	if ( !(state->timer = furi_timer_alloc(cbTimer, FuriTimerTypePeriodic, queue)) ) {
		ERROR(pwm_errs[(error = ERR_NO_TIMER)]);
		goto bail;
	}

	// ===== PWM =====
	// 10. Allocate a PWM actor called (uninspiringly) "pwm" ("Mr PWM" to you)
	if ( !(state->pwm = malloc(sizeof(*(state->pwm)))) ) {
		ERROR(pwm_errs[(error = ERR_NO_PWM)]);
		goto bail;
	}
	memset(state->pwm, 0, sizeof(*(state->pwm)));  // calloc() is not exported!

	INFO("INITIALISED");

	// ==================== Main event loop ====================

	if (run)  do {
		// Try to read a message from the queue
		// Our run-loop does not poll and is not "busy"
		//   but there is no "do not timeout"/"wait for message"
		//   so we need to use a large timeout and ignore timeout messages
		// --> furi/core/base.h
		FuriStatus  status = FuriStatusErrorTimeout;
		while ((status = furi_message_queue_get(queue, &msg, 5000)) == FuriStatusErrorTimeout) ;

		// Read failed
		if (status != FuriStatusOk) {
			switch (status) {
				case FuriStatusErrorTimeout:    DEBUG(pwm_errs[       DEBUG_QUEUE_TIMEOUT]);    break ;
				case FuriStatusError:           ERROR(pwm_errs[(error = ERR_QUEUE_RTOS)]);      goto bail ;
				case FuriStatusErrorResource:   ERROR(pwm_errs[(error = ERR_QUEUE_RESOURCE)]);  goto bail ;
				case FuriStatusErrorParameter:  ERROR(pwm_errs[(error = ERR_QUEUE_BADPRM)]);    goto bail ;
				case FuriStatusErrorNoMemory:   ERROR(pwm_errs[(error = ERR_QUEUE_NOMEM)]);     goto bail ;
				case FuriStatusErrorISR:        ERROR(pwm_errs[(error = ERR_QUEUE_ISR)]);       goto bail ;
				default:                        ERROR(pwm_errs[(error = ERR_QUEUE_UNK)]);       goto bail ;
			}

		// Read successful
		} else {
			// *** Try to lock the plugin state variables ***
			if ( !(state = (state_t*)acquire_mutex_block(&mutex)) ) {
				ERROR(pwm_errs[(error = ERR_MUTEX_BLOCK)]);
				goto bail;
			}

			// *** Handle events ***
			switch (msg.id) {
				case EVID_TICK:  // Timer events
					evTick(state);
					break;
				case EVID_KEY:   // Key events
					run = evKey(&msg, state, gui);
					break;
				default:         // Unknown event
					WARN("Unknown message.ID [%d]", msg.id);
					break;
			}

			// Update the GUI screen via the viewport
			view_port_update(vpp);

			// *** Try to release the plugin state variables ***
			if ( !release_mutex(&mutex, state) ) {
				ERROR(pwm_errs[(error = ERR_MUTEX_RELEASE)]);
				goto bail;
			}
		} // if (ok)
	} while (run);

	// ===== Game Over =====

	INFO("USER EXIT");

bail:
	// 10. PWM
	if (state->pwm)  free(state->pwm) ;

	// 9. Stop the timer
	if (state->timer) {
		(void)furi_timer_stop(state->timer);
		furi_timer_free(state->timer);
	}

	// 8. Detach the viewport
	gui_remove_view_port(gui, vpp);

	// 7. No need to unreqgister the callbacks
	//    ...they will go when the viewport is destroyed

	// 6. Destroy the viewport
	if (vpp) {
		view_port_enabled_set(vpp, false);
		view_port_free(vpp);
	}

	// 5. Free the mutex
	if (mutex.mutex)  delete_mutex(&mutex) ;

	// 4. Free up state pointer(s)

	// 3. Free the plugin state variables
	if (state)  free(state) ;

	// 2. Close the GUI
	furi_record_close("gui");

	// 1. Destroy the message queue
	if (queue)  furi_message_queue_free(queue) ;

	INFO("CLEAN EXIT ... Exit code: %d", error);
	LEAVE;
	return (int32_t)(error ? 255 : 0);  // It *seems* that the options are 0 for success, or 255 for failure
}
