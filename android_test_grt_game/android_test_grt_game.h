#include "gpk_error.h"

#ifndef GPK_ANDROID_INTENTS_H_23874293847
#define GPK_ANDROID_INTENTS_H_23874293847

class android_test_grt_game {
public:
	const char *				getPlatformABI				();
};

extern "C" {
	::gpk::error_t				gpk_appDelete					(void * app);
	::gpk::error_t				gpk_appCreate					(void** app);
	::gpk::error_t				gpk_appCleanup					(void * app);
	::gpk::error_t				gpk_appSetup					(void * app);
	::gpk::error_t				gpk_appDraw						(void * app);
	::gpk::error_t				gpk_appUpdate					(void * app);
} // extern "C"
#endif // GPK_ANDROID_INTENTS_H_23874293847