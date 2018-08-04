#include "android_intents.h"
#include "android_native_app_glue.h"

namespace NativeIntents {
	namespace detail {
		static constexpr char intent[] = "android/content/Intent";
		static constexpr char activity[] = "android/app/Activity";
		static constexpr char uri[] = "android/net/Uri";

		static constexpr char contructor[] = "<init>";
		static constexpr char set_action[] = "setAction";
		static constexpr char set_data[] = "setData";
	}

	static inline const char * get_action_text(Actions act) {
		static constexpr char act_view[] = "android.intent.action.VIEW";

		switch (act)
		{
			case Actions::View: return act_view;
		}

		return nullptr;
	}

	EnvLocker::EnvLocker	(struct android_app * app): app(app)	{ app->activity->vm->AttachCurrentThread(&env, NULL); }
	EnvLocker::~EnvLocker	()										{ app->activity->vm->DetachCurrentThread(); }
	static jobject alloc_intent(JNIEnv * env) {
		jclass intent_class = env->FindClass(detail::intent);
		if (!intent_class || env->ExceptionCheck()) return nullptr;

		jobject intent = env->AllocObject(intent_class);
		if (!intent || env->ExceptionCheck()) return nullptr;

		jmethodID contructor = env->GetMethodID(intent_class, detail::contructor, "()V");
		if (!contructor || env->ExceptionCheck()) return nullptr;

		env->CallVoidMethod(intent, contructor);
		return intent;
	}

	static jobject set_action(jobject intent, jstring act, JNIEnv * env) {
		jclass intent_class = env->FindClass(detail::intent);
		if (!intent_class || env->ExceptionCheck()) return nullptr;

		jmethodID set_act = env->GetMethodID(intent_class, detail::set_action, "(Ljava/lang/String;)Landroid/content/Intent;");
		if (!set_act || env->ExceptionCheck()) return nullptr;

		env->CallObjectMethod(intent, set_act, act);
		if (env->ExceptionCheck()) return nullptr;

		return intent;
	}

	static jobject parse_uri(const char *uri, JNIEnv * env){
		jclass uri_class = env->FindClass(detail::uri);
		jmethodID parse = env->GetStaticMethodID(uri_class, "parse", "(Ljava/lang/String;)Landroid/net/Uri;");
		if (!parse || env->ExceptionCheck()) return nullptr;

		return env->CallStaticObjectMethod(uri_class, parse, env->NewStringUTF(uri));
	}

	jobject create_intent(Actions action, EnvLocker & locker){
		auto act = get_action_text(action);
		if (!act) return nullptr;

		auto env = locker.get_env();

		auto intent = alloc_intent(env);
		if (!intent) return nullptr;

		auto jav_str = env->NewStringUTF(act);
		return set_action(intent, jav_str, env);
	}

	bool set_uri(jobject intent, const char *uri, EnvLocker & locker){
		auto env = locker.get_env();

		auto java_uri = parse_uri(uri, env);
		if (!java_uri) return false;

		jclass intent_class = env->FindClass(detail::intent);
		if (!intent_class || env->ExceptionCheck()) return false;

		jmethodID set_uri = env->GetMethodID(intent_class, detail::set_data, "(Landroid/net/Uri;)Landroid/content/Intent;");
		if (!set_uri || env->ExceptionCheck()) return false;

		env->CallObjectMethod(intent, set_uri, java_uri);
		if (env->ExceptionCheck()) return false;

		return intent;
	}

	bool run_intent(jobject intent, EnvLocker & locker){
		auto env = locker.get_env();
		auto app = locker.get_app();

		jclass activity = env->FindClass(detail::activity);
		jmethodID start_activity = env->GetMethodID(activity, "startActivity", "(Landroid/content/Intent;)V");
		if (!start_activity || env->ExceptionCheck()) return false;

		jobject context = app->activity->clazz;
		env->CallVoidMethod(context, start_activity, intent);

		return env->ExceptionCheck();
	}

}