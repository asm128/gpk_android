#include "gpk_error.h"
#include "gpk_module.h"

#ifndef GPK_ANDROID_INTENTS_H_23874293847
#define GPK_ANDROID_INTENTS_H_23874293847

class android_test_grt_game {
public:
	const char *					getPlatformABI					();
};

extern "C" 
{
	GPK_DECLARE_MODULE_FUNCTION(gpk_appDelete		, void * app);
	GPK_DECLARE_MODULE_FUNCTION(gpk_appCreate		, void** app);
	GPK_DECLARE_MODULE_FUNCTION(gpk_appCleanup		, void * app);
	GPK_DECLARE_MODULE_FUNCTION(gpk_appSetup		, void * app);
	GPK_DECLARE_MODULE_FUNCTION(gpk_appDraw			, void * app);
	GPK_DECLARE_MODULE_FUNCTION(gpk_appUpdate		, void * app);
} // extern "C"
#endif // GPK_ANDROID_INTENTS_H_23874293847