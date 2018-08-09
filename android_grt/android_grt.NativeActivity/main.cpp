// Copyright (C) 2010 The Android Open Source Project
// Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and limitations under the License.

#include <EGL/egl.h>
#include <GLES/gl.h>
#include <android/sensor.h>
#include <android/log.h>
#include <memory>
#include <dlfcn.h>
#include <include/android/hardware_buffer.h>


#include "android_native_app_glue.h"

#include "gpk_bitmap_target.h"
#include "gpk_image.h"
#include "gpk_color.h"
#include "gpk_runtime.h"
#include "gpk_module.h"

#include "android_intents.h"

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "AndroidProject1.NativeActivity", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "AndroidProject1.NativeActivity", __VA_ARGS__))

// Our saved state data.
struct saved_state {
	float				angle;
	int32_t				x;
	int32_t				y;
};

// Shared state for our app.
struct engine {
	struct android_app	* app;
	ASensorManager		* sensorManager;
	const ASensor		* accelerometerSensor;
	ASensorEventQueue	* sensorEventQueue;
	int					animating;
	EGLDisplay			display;
	EGLSurface			surface;
	EGLContext			context;
	int32_t				width;
	int32_t				height;
	struct saved_state	state;
};

//// Initialize an EGL context for the current display.
//static int				engine_init_display			(struct engine* engine)			{
//	// initialize OpenGL ES and EGL
//
//	// Here specify the attributes of the desired configuration. Below, we select an EGLConfig with at least 8 bits per color component compatible with on-screen windows.
//	const EGLint attribs[] = 
//		{ EGL_SURFACE_TYPE, EGL_WINDOW_BIT
//		, EGL_BLUE_SIZE	, 8
//		, EGL_GREEN_SIZE, 8
//		, EGL_RED_SIZE	, 8
//		, EGL_NONE
//		};
//	EGLint							w, h, format;
//	EGLint							numConfigs;
//	EGLConfig						config;
//	EGLSurface						surface;
//	EGLContext						context;
//	EGLDisplay						display					= eglGetDisplay(EGL_DEFAULT_DISPLAY);
//
//	eglInitialize		(display, 0, 0);
//	eglChooseConfig		(display, attribs, &config, 1, &numConfigs);		// Here, the application chooses the configuration it desires. In this sample, we have a very simplified selection process, where we pick the first EGLConfig that matches our criteria.
//	eglGetConfigAttrib	(display, config, EGL_NATIVE_VISUAL_ID, &format);		// EGL_NATIVE_VISUAL_ID is an attribute of the EGLConfig that is guaranteed to be accepted by ANativeWindow_setBuffersGeometry(). As soon as we picked a EGLConfig, we can safely reconfigure the ANativeWindow buffers to match, using EGL_NATIVE_VISUAL_ID. 
//
//	ANativeWindow_setBuffersGeometry(engine->app->window, 0, 0, format);
//
//	surface = eglCreateWindowSurface(display, config, engine->app->window, NULL);
//	context = eglCreateContext		(display, config, NULL, NULL);
//
//	if (eglMakeCurrent(display, surface, surface, context) == EGL_FALSE) {
//		LOGW("Unable to eglMakeCurrent");
//		return -1;
//	}
//
//	eglQuerySurface(display, surface, EGL_WIDTH, &w);
//	eglQuerySurface(display, surface, EGL_HEIGHT, &h);
//
//	engine->display		= display;
//	engine->context		= context;
//	engine->surface		= surface;
//	engine->width		= w;
//	engine->height		= h;
//	engine->state.angle	= 0;
//
//	// Initialize GL state.
//	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
//	glEnable(GL_CULL_FACE);
//	glShadeModel(GL_SMOOTH);
//	glDisable(GL_DEPTH_TEST);
//
//	return 0;
//}

//// Just the current frame in the display.
//static void engine_draw_frame(struct engine* engine) {
//	if (engine->display == NULL) // No display.
//		return;
//
//	// Just fill the screen with a color.
//	glClearColor(((float)engine->state.x) / engine->width, engine->state.angle, ((float)engine->state.y) / engine->height, 1);
//	glClear(GL_COLOR_BUFFER_BIT);
//
//	//glDrawPixels(engine->width,  engine->height, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8, target.View.begin());
//
//	//glPixelStorei();
//	eglSwapBuffers(engine->display, engine->surface);
//
//}

//// Tear down the EGL context currently associated with the display.
//static void engine_term_display(struct engine* engine) {
//	if (engine->display != EGL_NO_DISPLAY) {
//		eglMakeCurrent(engine->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
//		if (engine->context != EGL_NO_CONTEXT) 
//			eglDestroyContext(engine->display, engine->context);
//		
//		if (engine->surface != EGL_NO_SURFACE) 
//			eglDestroySurface(engine->display, engine->surface);
//		
//		eglTerminate(engine->display);
//	}
//	engine->animating = 0;
//	engine->display = EGL_NO_DISPLAY;
//	engine->context = EGL_NO_CONTEXT;
//	engine->surface = EGL_NO_SURFACE;
//}

// Process the next input event.
static int32_t engine_handle_input(struct android_app* app, AInputEvent* event) {
	struct engine* engine = (struct engine*)app->userData;
	if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
		engine->state.x = AMotionEvent_getX(event, 0);
		engine->state.y = AMotionEvent_getY(event, 0);
		info_printf("Input event {x: %u, y: %u}.", engine->state.x, engine->state.y);
		return 1;
	}
	return 0;
}

// Process the next main command.
static void engine_handle_cmd(struct android_app* app, int32_t cmd) {
	struct engine										* engine						= (struct engine*)app->userData;
	switch (cmd) {
	case APP_CMD_SAVE_STATE: // The system has asked us to save our current state.  Do so.
		engine->app->savedState							= malloc(sizeof(struct saved_state));
		*((struct saved_state*)engine->app->savedState) = engine->state;
		engine->app->savedStateSize						= sizeof(struct saved_state);
		break;
	case APP_CMD_INIT_WINDOW: 
		if (engine->app->window != NULL) {	// The window is being shown, get it ready.
			//engine_init_display(engine);
			//engine_draw_frame(engine);
		}
		break;
	case APP_CMD_TERM_WINDOW:
		//engine_term_display(engine);	 // The window is being hidden or closed, clean it up.
		break;
	case APP_CMD_GAINED_FOCUS: 
		if (engine->accelerometerSensor != NULL) {	// When our app gains focus, we start monitoring the accelerometer.
			ASensorEventQueue_enableSensor(engine->sensorEventQueue, engine->accelerometerSensor);
			ASensorEventQueue_setEventRate(engine->sensorEventQueue, engine->accelerometerSensor, (1000L / 60) * 1000);	// We'd like to get 60 events per second (in us).
		}
		break;
	case APP_CMD_LOST_FOCUS: 
		if (engine->accelerometerSensor != NULL)	// When our app loses focus, we stop monitoring the accelerometer. This is to avoid consuming battery while not being used.
			ASensorEventQueue_disableSensor(engine->sensorEventQueue, engine->accelerometerSensor);
		// Also stop animating.
		engine->animating = 0;
		//engine_draw_frame(engine);
		break;
	}
}

bool native_open_url(struct android_app* app, const char *url)
{
	using namespace NativeIntents;

	EnvLocker lock{ app };

	auto intent = create_intent(Actions::View, lock);
	if (!intent) return false;

	if (!set_uri(intent, url, lock)) return false;

	return run_intent(intent, lock);
}

// This is the main entry point of a native application that is using android_native_app_glue.  It runs in its own thread, with its own event loop for receiving input events and doing other things.
void android_main(struct android_app* state) {
	
	struct engine							engine;

	memset(&engine, 0, sizeof(engine));
	state->userData						= &engine;
	state->onAppCmd						= engine_handle_cmd;
	state->onInputEvent					= engine_handle_input;
	engine.app							= state;

	// Prepare to monitor accelerometer
	engine.sensorManager				= ASensorManager_getInstance();
	engine.accelerometerSensor			= ASensorManager_getDefaultSensor(engine.sensorManager, ASENSOR_TYPE_ACCELEROMETER);
	engine.sensorEventQueue				= ASensorManager_createEventQueue(engine.sensorManager, state->looper, LOOPER_ID_USER, NULL, NULL);

	if (state->savedState != NULL)			// We are starting with a previous saved state; restore from it.
		engine.state						= *(struct saved_state*)state->savedState;

	engine.animating					= 1;

	void									* handleModule				= GPK_LOAD_MODULE(".\\libandroid_test_grt_game.so");
	if(handleModule) {
		void* sym = dlsym(handleModule, "gpk_appCreate");
		if(sym)
			native_open_url(state, "http://www.example.com/q?symbol");
		else
			native_open_url(state, "http://www.example.com/q?module");
		GPK_FREE_MODULE(handleModule);
	}

	::gpk::SImage<::gpk::SColorBGRA>		target;

	// loop waiting for stuff to do.
	while (1) {
		// Read all pending events.
		int										ident;
		int										events;
		struct android_poll_source				* source;

		// If not animating, we will block forever waiting for events.
		// If animating, we loop until all events are read, then continue to draw the next frame of animation.
		while ((ident = ALooper_pollAll(engine.animating ? 0 : -1, NULL, &events,(void**)&source)) >= 0) {
			if (source != NULL)		// Process this event.
				source->process(state, source);

			if (ident == LOOPER_ID_USER) {	// If a sensor has data, process it now.
				if (engine.accelerometerSensor != NULL) {
					ASensorEvent event;
					while (ASensorEventQueue_getEvents(engine.sensorEventQueue, &event, 1) > 0) {
						//LOGI("accelerometer: x=%f y=%f z=%f", event.acceleration.x, event.acceleration.y, event.acceleration.z);
					}
				}
			}

			if (state->destroyRequested != 0) {	// Check if we are exiting.
				//engine_term_display(&engine);
				return;
			}
		}

		if (engine.animating) {
			// Done with events; draw next animation frame.
			engine.state.angle								+= 1.0f / 60.0f;// .01f;
			if (engine.state.angle > 1) 
				engine.state.angle								= 0;

			//engine_draw_frame(&engine);	// Drawing is throttled to the screen update rate, so there is no need to do timing here.

			pthread_mutex_lock(&state->mutex);
			if(state->window) {
				const ::gpk::SCoord2<uint32_t>						targetSizeOriginal					= {(uint32_t)ANativeWindow_getWidth(state->window), (uint32_t)ANativeWindow_getHeight(state->window)};
				::gpk::SCoord2<uint32_t>							targetSize							= {};
				targetSize.x									= ::gpk::max(targetSizeOriginal.x, targetSizeOriginal.y);
				targetSize.y									= ::gpk::min(targetSizeOriginal.x, targetSizeOriginal.y);
				target.resize(targetSize.x, targetSize.y, ::gpk::SColorBGRA{});
				::gpk::drawLine(target.View, ::gpk::SColorBGRA{0x00, 0x00, 0xFF, 0xFF}, ::gpk::SLine2D<uint32_t>{{}, target.metrics() * (double)engine.state.angle});
			
				ANativeWindow_Buffer								buffer;
				ARect												dirtyBounds							= {0, 0, (int32_t)target.metrics().x, (int32_t)target.metrics().y};
			
				if(0 == ANativeWindow_lock(state->window, &buffer, &dirtyBounds)) {
					switch(buffer.format) {
					case  AHARDWAREBUFFER_FORMAT_R8G8B8X8_UNORM	: 
					case  AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM	: 
						if(buffer.height == target.metrics().y)
							for(uint32_t y = 0, maxY = ::gpk::min((uint32_t)buffer.height, target.metrics().y); y < maxY; ++y)
							for(uint32_t x = 0, maxX = ::gpk::min((uint32_t)buffer.width , target.metrics().x); x < maxX; ++x) {
								::gpk::SColorBGRA	orig = target[y][x];
								uint32_t			idxTargetByte = y * (buffer.stride * sizeof(::gpk::SColorRGBA)) + sizeof(::gpk::SColorRGBA) * x;
								(*(::gpk::SColorRGBA*)&((char*)buffer.bits)[idxTargetByte]) = {orig.r, orig.g, orig.b, orig.a};
//								memcpy(&((char*)buffer.bits)[y * (buffer.stride * sizeof(::gpk::SColorBGRA))], target[y].begin(), ::gpk::min((uint32_t)buffer.stride, target.metrics().x) * sizeof(::gpk::SColorBGRA));
							}

						break;
					case  AHARDWAREBUFFER_FORMAT_R8G8B8_UNORM	: 
						for(uint32_t iPix = 0, coutnPix = target.Texels.size(); iPix < coutnPix; ++iPix) {
							::gpk::SColorBGRA								& srcPix							= target.Texels[iPix];
							((::gpk::SColorBGR*)buffer.bits)[iPix]		= {srcPix.b, srcPix.g, srcPix.r};
						}
						break;
					default:
						error_printf("Unsupported hardware buffer: %X.", (uint32_t)buffer.format);
					}
					buffer.bits;
					ANativeWindow_unlockAndPost(state->window);
				}
			}
			pthread_mutex_unlock(&state->mutex);
		}
	}
}
