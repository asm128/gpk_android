#ifndef GPK_ANDROID_INTENTS_H_23874293847
#define GPK_ANDROID_INTENTS_H_23874293847

#include <jni.h>

struct android_app;

namespace NativeIntents {
	enum class Actions {
		View							= 0
	};

	class EnvLocker {
		android_app						* app;
		JNIEnv							* env;
	public:
										~EnvLocker					();
		explicit						EnvLocker					(struct android_app *app);
										EnvLocker					(EnvLocker &)					= delete;
		JNIEnv							* get_env					()	const	{ return env; }
		struct android_app				* get_app					()	const	{ return app; }
	};

	jobject							create_intent				(Actions action, EnvLocker & locker);
	bool							set_uri						(jobject intent, const char *uri, EnvLocker & locker);
	bool							run_intent					(jobject intent, EnvLocker & locker);
	
}

#endif // GPK_ANDROID_INTENTS_H_23874293847