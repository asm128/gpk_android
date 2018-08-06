#include "android_intents.h"
#include "android_native_app_glue.h"
#include "gpk_log.h"

namespace NativeIntents {
	namespace detail {
		static constexpr	const char			intent		[]				= "android/content/Intent";
		static constexpr	const char			activity	[]				= "android/app/Activity";
		static constexpr	const char			uri			[]				= "android/net/Uri";

		static constexpr	const char			contructor	[]				= "<init>";
		static constexpr	const char			set_action	[]				= "setAction";
		static constexpr	const char			set_data	[]				= "setData";
	}

	static inline		const char *		get_action_text				(Actions act)											{
		static constexpr	const char				act_view[]					= "android.intent.action.VIEW";
		switch (act) {
		case Actions::View: return act_view;
		}
		return nullptr;
	}

											EnvLocker::EnvLocker		(struct android_app * app): app(app)					{ app->activity->vm->AttachCurrentThread(&env, NULL); }
											EnvLocker::~EnvLocker		()														{ app->activity->vm->DetachCurrentThread(); }
	static				jobject				alloc_intent				(JNIEnv * env)											{
		jclass										intent_class				= env->FindClass(detail::intent);								retnul_error_if(!intent_class	|| env->ExceptionCheck(), "%s", "");
		jobject										intent						= env->AllocObject(intent_class);								retnul_error_if(!intent			|| env->ExceptionCheck(), "%s", "");
		jmethodID									constructor					= env->GetMethodID(intent_class, detail::contructor, "()V");	retnul_error_if(!constructor	|| env->ExceptionCheck(), "%s", "");
		env->CallVoidMethod(intent, constructor);
		return intent;
	}

	static				jobject				set_action					(jobject intent, jstring act, JNIEnv * env)				{
		jclass										intent_class				= env->FindClass(detail::intent);	
		retnul_error_if(!intent_class	|| env->ExceptionCheck(), "%s", "");
		jmethodID									set_act						= env->GetMethodID(intent_class, detail::set_action, "(Ljava/lang/String;)Landroid/content/Intent;");
		if (!set_act || env->ExceptionCheck()) return nullptr;

		env->CallObjectMethod(intent, set_act, act);
		if (env->ExceptionCheck()) return nullptr;

		return intent;
	}

	static				jobject				parse_uri					(const char *uri, JNIEnv * env)							{
		jclass										uri_class					= env->FindClass(detail::uri);
		jmethodID									parse						= env->GetStaticMethodID(uri_class, "parse", "(Ljava/lang/String;)Landroid/net/Uri;");
		if (!parse || env->ExceptionCheck()) return nullptr;
		return env->CallStaticObjectMethod(uri_class, parse, env->NewStringUTF(uri));
	}

						jobject				create_intent				(Actions action, EnvLocker & locker)					{
		auto										act							= get_action_text(action);
		if (!act) return nullptr;

		JNIEnv										* env						= locker.get_env();
		jobject										intent						= alloc_intent(env);
		retnul_error_if(!intent|| env->ExceptionCheck(), "%s", "");

		auto										jav_str						= env->NewStringUTF(act);
		return set_action(intent, jav_str, env);
	}

						bool				set_uri						(jobject intent, const char *uri, EnvLocker & locker)	{
		JNIEnv										* env						= locker.get_env();
		jobject										java_uri					= parse_uri(uri, env);
		if (!java_uri) return false;		

		jclass										intent_class				= env->FindClass(detail::intent);
		if (!intent_class || env->ExceptionCheck()) return false;

		jmethodID									set_uri						= env->GetMethodID(intent_class, detail::set_data, "(Landroid/net/Uri;)Landroid/content/Intent;");
		if (!set_uri || env->ExceptionCheck()) return false;

		env->CallObjectMethod(intent, set_uri, java_uri);
		if (env->ExceptionCheck()) return false;
		return intent;
	}

						bool				run_intent					(jobject intent, EnvLocker & locker)					{
		JNIEnv										* env						= locker.get_env();
		android_app									* app						= locker.get_app();
		jclass										activity					= env->FindClass(detail::activity);
		jmethodID									start_activity				= env->GetMethodID(activity, "startActivity", "(Landroid/content/Intent;)V");
		if (!start_activity || env->ExceptionCheck()) return false;

		jobject										context						= app->activity->clazz;
		env->CallVoidMethod(context, start_activity, intent);
		return env->ExceptionCheck();
	}
}