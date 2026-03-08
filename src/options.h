#ifdef PC
	#define K_QUIT		SDLK_q
	#define K_QUIT2		SDLK_DELETE
	#define K_NEW		SDLK_BACKSPACE
	#define K_ZOOM_IN	SDLK_1
	//#define K_ZOOM_IN2
	#define K_ZOOM_OUT	SDLK_2
	//#define K_ZOOM_OUT2
	#define K_MODE_CLICK	SDLK_RETURN
	#define K_MODE_CLICK2	SDLK_r
	#define K_MODE_FLAG	SDLK_e
	#define K_MODE_QUEST	SDLK_w
	#define K_MODE_QUEST2	SDLK_PERIOD
	#define K_0		SDLK_0
	#define K_1		SDLK_1
	#define K_2		SDLK_2
	#define K_3		SDLK_3
	#define K_4		SDLK_4
	#define K_5		SDLK_5
	#define K_6		SDLK_6
	#define K_7		SDLK_7
	#define K_8		SDLK_8
	#define K_9		SDLK_9
#else
	#define K_QUIT		KEYCODE_POWER_CLEAR // POWER_Clear
	//#define K_QUIT2
	#define K_NEW		KEYCODE_BACKSPACE // BACKSPACE
	#define K_ZOOM_IN	KEYCODE_PLUS // Plus
	#define K_ZOOM_IN2	KEYCODE_1 // 1
	#define K_ZOOM_OUT	KEYCODE_MINUS // Minus
	#define K_ZOOM_OUT2	KEYCODE_2 // 2
	#define K_MODE_CLICK	KEYCODE_EXE // EXE
	//#define K_MODE_CLICK2
	#define K_MODE_FLAG	KEYCODE_EXP // EXP
	#define K_MODE_QUEST	KEYCODE_DOT // DOT
	//#define K_MODE_QUEST2
	#define K_0		KEYCODE_0 //The number keys 0-9
	#define K_1		KEYCODE_1
	#define K_2		KEYCODE_2
	#define K_3		KEYCODE_3
	#define K_4		KEYCODE_4
	#define K_5		KEYCODE_5
	#define K_6		KEYCODE_6
	#define K_7		KEYCODE_7
	#define K_8		KEYCODE_8
	#define K_9		KEYCODE_9
#endif
