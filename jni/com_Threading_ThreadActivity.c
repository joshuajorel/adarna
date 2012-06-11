#include <android/log.h>
#include <stdlib.h>
#include <jni.h>
#include <android/sensor.h>
#include <android/looper.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "TestJNIActivity", __VA_ARGS__))
#define LOOPER_ID 1
#define SAMP_PER_SEC 100
#define SAMPLE_LIMIT 1024
#define ACC_SAMP_FREQ 100
#define GYR_SAMP_FREQ 100

int64_t lastAccTime = 0;
int64_t lastGyrTime = 0;
FILE* accF;
FILE* gyrF;

int accSampleCount = 0;
int gyrSampleCount = 0;
int accFlag = 0;
int gyrFlag = 0;

ASensorEventQueue* accQueue;
ASensorEventQueue* gyrQueue;

static int get_acc_events(int fd, int events, void* data);
static int get_gyr_events(int fd, int events, void* data);

JNIEXPORT jstring JNICALL Java_com_Threading_ThreadActivity_goThread
  (JNIEnv *env, jclass class){
	LOGI("Sensor Data Start");
	return (*env)->NewStringUTF(env, "Acquiring Sensor Data");
}

JNIEXPORT void JNICALL Java_com_Threading_ThreadActivity_threader
  (JNIEnv *env, jclass class){
	ASensorEvent accEvent;
	ASensorEvent gyrEvent;
	ASensorManager* accSensorManager;
	ASensorManager* gyrSensorManager;
	const ASensor *accSensor;
	const ASensor *gyrSensor;
	void *acc_data = malloc(1000);
	void *gyr_data = malloc(1000);
	accF = fopen("/sdcard/acc-samp.csv", "w");
	gyrF = fopen("/sdcard/gyr-samp.csv", "w");
	if (accF == NULL) {
		LOGI(" ERROR : file does not exist");
	}
	if (gyrF == NULL) {
			LOGI(" ERROR : file does not exist");
		}

	LOGI("sensorValue() - ALooper_forThread");

	ALooper* accLooper = ALooper_forThread();
	ALooper* gyrLooper = ALooper_forThread();

	if(accLooper == NULL){
		accLooper = ALooper_prepare(ALOOPER_PREPARE_ALLOW_NON_CALLBACKS);
	}

	if(gyrLooper == NULL){
			gyrLooper = ALooper_prepare(ALOOPER_PREPARE_ALLOW_NON_CALLBACKS);
	}

	accSensorManager = ASensorManager_getInstance();
	gyrSensorManager = ASensorManager_getInstance();
	accSensor = ASensorManager_getDefaultSensor(accSensorManager, ASENSOR_TYPE_ACCELEROMETER);
	gyrSensor = ASensorManager_getDefaultSensor(gyrSensorManager, ASENSOR_TYPE_GYROSCOPE);

	accQueue = ASensorManager_createEventQueue(accSensorManager, accLooper, 3, get_acc_events, acc_data);
	gyrQueue = ASensorManager_createEventQueue(gyrSensorManager, gyrLooper, 3, get_gyr_events, gyr_data);

	ASensorEventQueue_enableSensor(accQueue, accSensor);
	ASensorEventQueue_enableSensor(gyrQueue, gyrSensor);
	int a = ASensor_getMinDelay(accSensor);
	int b = ASensor_getMinDelay(gyrSensor);
	LOGI("min-delay: %d", a);
	LOGI("min-delay: %d", b);

	ASensorEventQueue_setEventRate(accQueue, accSensor, (1000L/ACC_SAMP_FREQ)*1000);
	ASensorEventQueue_setEventRate(gyrQueue, gyrSensor, (1000L/GYR_SAMP_FREQ)*1000);

	LOGI("sensorValue() - Start");
	if ((accSampleCount == SAMPLE_LIMIT) && !accFlag){
		free(acc_data);
	}

	if ((gyrSampleCount == SAMPLE_LIMIT) && !gyrFlag){
		free(gyr_data);
	}
}

static int get_acc_events(int fd, int events, void* data){
	ASensorEvent event;
	while(ASensorEventQueue_getEvents(accQueue, &event, 1) >0 && (accSampleCount < SAMPLE_LIMIT)){
		if(event.type == ASENSOR_TYPE_ACCELEROMETER){
			//LOGI("[%d][%f][%f][%f][%f]", accSampleCount, ((double)(event.timestamp-lastAccTime))/1000000000.0, event.acceleration.x,event.acceleration.y,event.acceleration.z);
			fprintf(accF,"%f; ",((double)(event.timestamp-lastAccTime))/1000000000.0);
			lastAccTime = event.timestamp;
			accSampleCount++;
		}

		if(accSampleCount == SAMPLE_LIMIT){
			LOGI("ACC SAMPLING FINISHED");
			fclose(accF);
		}

	}
	return 1;
}

static int get_gyr_events(int fd, int events, void* data){
	ASensorEvent event;
	while(ASensorEventQueue_getEvents(gyrQueue, &event, 1) >0 && (gyrSampleCount < SAMPLE_LIMIT)){
		if(event.type == ASENSOR_TYPE_GYROSCOPE){
			//LOGI("[%d][%f][%f][%f][%f]", gyrSampleCount, ((double)(event.timestamp-lastGyrTime))/1000000000.0, event.acceleration.x,event.acceleration.y,event.acceleration.z);
			fprintf(gyrF,"%f; ",((double)(event.timestamp-lastGyrTime))/1000000000.0);
			lastGyrTime = event.timestamp;
			gyrSampleCount++;
		}

		if(gyrSampleCount == SAMPLE_LIMIT){
			LOGI("GYR SAMPLING FINISHED");
			fclose(gyrF);
		}

	}
	return 1;
}
