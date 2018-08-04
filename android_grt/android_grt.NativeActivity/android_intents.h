#pragma once

#include <jni.h>

struct android_app;
namespace NativeIntents {
	enum class Actions {
		View = 0
	};

	class EnvLocker {
	public:
		explicit EnvLocker(struct android_app *app);
		EnvLocker(EnvLocker &) = delete;
		~EnvLocker();

		JNIEnv *get_env() const { return env;  }
		struct android_app *get_app() const { return app; }

	private:
		struct android_app *app;
		JNIEnv *env;

	};

	jobject create_intent(Actions action, EnvLocker & locker);
	bool set_uri(jobject intent, const char *uri, EnvLocker & locker);
	bool run_intent(jobject intent, EnvLocker & locker);
	
}
