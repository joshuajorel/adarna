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
#define MAG_SAMP_FREQ 100
#define BAR_SAMP_FREQ 5
#define BAR_LIMIT 60

int64_t lastAccTime = 0;
int64_t lastGyrTime = 0;
int64_t lastMagTime = 0;
int64_t lastBarTime = 0;
FILE* accF;
FILE* gyrF;
FILE* magF;
FILE* barF;

int accSampleCount = 0;
int gyrSampleCount = 0;
int magSampleCount = 0;
int barSampleCount = 0;

ASensorEventQueue* accQueue;
ASensorEventQueue* gyrQueue;
ASensorEventQueue* magQueue;
ASensorEventQueue* barQueue;

static int get_acc_events(int fd, int events, void* data);
static int get_gyr_events(int fd, int events, void* data);
static int get_mag_events(int fd, int events, void* data);
static int get_bar_events(int fd, int events, void* data);

JNIEXPORT jstring JNICALL Java_com_Threading_ThreadActivity_goThread
  (JNIEnv *env, jclass class){
	LOGI("Sensor Data Start");
	return (*env)->NewStringUTF(env, "Acquiring Sensor Data");
}

JNIEXPORT void JNICALL Java_com_Threading_ThreadActivity_threader
  (JNIEnv *env, jclass class){
	ASensorEvent accEvent;
	ASensorEvent gyrEvent;
	ASensorEvent magEvent;
	ASensorEvent barEvent;
	ASensorManager* accSensorManager;
	ASensorManager* gyrSensorManager;
	ASensorManager* magSensorManager;
	ASensorManager* barSensorManager;

	const ASensor *accSensor;
	const ASensor *gyrSensor;
	const ASensor *magSensor;
	const ASensor *barSensor;

	void *acc_data = malloc(1000);
	void *gyr_data = malloc(1000);
	void *mag_data = malloc(1000);
	void *bar_data = malloc(1000);

	accF = fopen("/sdcard/acc-samp.csv", "w");
	gyrF = fopen("/sdcard/gyr-samp.csv", "w");
	magF = fopen("/sdcard/mag-samp.csv", "w");
	barF = fopen("/sdcard/bar-samp.csv", "w");

	if (accF == NULL) {
		LOGI(" ERROR : acc does not exist");
	}

	if (gyrF == NULL) {
		LOGI(" ERROR : gyr does not exist");
	}

	if (magF == NULL) {
		LOGI(" ERROR : mag does not exist");
	}

	if (barF == NULL) {
		LOGI(" ERROR : bar does not exist");
	}

	LOGI("sensorValue() - ALooper_forThread");

	ALooper* accLooper = ALooper_forThread();
	ALooper* gyrLooper = ALooper_forThread();
	ALooper* magLooper = ALooper_forThread();
	ALooper* barLooper = ALooper_forThread();

	if(accLooper == NULL){
		accLooper = ALooper_prepare(ALOOPER_PREPARE_ALLOW_NON_CALLBACKS);
	}

	if(gyrLooper == NULL){
		gyrLooper = ALooper_prepare(ALOOPER_PREPARE_ALLOW_NON_CALLBACKS);
	}

	if(magLooper == NULL){
		magLooper = ALooper_prepare(ALOOPER_PREPARE_ALLOW_NON_CALLBACKS);
	}

	if(barLooper == NULL){
		barLooper = ALooper_prepare(ALOOPER_PREPARE_ALLOW_NON_CALLBACKS);
	}

	accSensorManager = ASensorManager_getInstance();
	gyrSensorManager = ASensorManager_getInstance();
	magSensorManager = ASensorManager_getInstance();
	barSensorManager = ASensorManager_getInstance();

	accSensor = ASensorManager_getDefaultSensor(accSensorManager, ASENSOR_TYPE_ACCELEROMETER);
	gyrSensor = ASensorManager_getDefaultSensor(gyrSensorManager, ASENSOR_TYPE_GYROSCOPE);
	magSensor = ASensorManager_getDefaultSensor(magSensorManager, ASENSOR_TYPE_MAGNETIC_FIELD);
	barSensor = ASensorManager_getDefaultSensor(barSensorManager, 6);

	accQueue = ASensorManager_createEventQueue(accSensorManager, accLooper, 3, get_acc_events, acc_data);
	gyrQueue = ASensorManager_createEventQueue(gyrSensorManager, gyrLooper, 3, get_gyr_events, gyr_data);
	magQueue = ASensorManager_createEventQueue(magSensorManager, magLooper, 3, get_mag_events, mag_data);
	barQueue = ASensorManager_createEventQueue(barSensorManager, barLooper, 3, get_bar_events, bar_data);

	ASensorEventQueue_enableSensor(accQueue, accSensor);
	ASensorEventQueue_enableSensor(gyrQueue, gyrSensor);
	ASensorEventQueue_enableSensor(magQueue, magSensor);
	ASensorEventQueue_enableSensor(barQueue, barSensor);

	int a = ASensor_getMinDelay(accSensor);
	int b = ASensor_getMinDelay(gyrSensor);
	int c = ASensor_getMinDelay(magSensor);
	int d = ASensor_getMinDelay(barSensor);

	LOGI("min-delay: %d", a);
	LOGI("min-delay: %d", b);
	LOGI("min-delay: %d", c);
	LOGI("min-delay: %d", d);

	ASensorEventQueue_setEventRate(accQueue, accSensor, (1000L/ACC_SAMP_FREQ)*1000);
	ASensorEventQueue_setEventRate(gyrQueue, gyrSensor, (1000L/GYR_SAMP_FREQ)*1000);
	ASensorEventQueue_setEventRate(magQueue, magSensor, (1000L/MAG_SAMP_FREQ)*1000);
	ASensorEventQueue_setEventRate(barQueue, barSensor, (1000L/BAR_SAMP_FREQ)*1000);

	LOGI("sensorValue() - Start");

	if (accSampleCount == SAMPLE_LIMIT){
		free(acc_data);
	}

	if (gyrSampleCount == SAMPLE_LIMIT){
		free(gyr_data);
	}

	if (magSampleCount == SAMPLE_LIMIT){
		free(mag_data);
	}

	if (barSampleCount == SAMPLE_LIMIT){
		free(bar_data);
	}
}

static int get_acc_events(int fd, int events, void* data){
	ASensorEvent event;
	while(ASensorEventQueue_getEvents(accQueue, &event, 1) >0 && (accSampleCount < SAMPLE_LIMIT)){
		if(event.type == ASENSOR_TYPE_ACCELEROMETER){
			//LOGI("[%d][%f][%f][%f][%f]", accSampleCount, ((double)(event.timestamp-lastAccTime))/1000000000.0, event.acceleration.x,event.acceleration.y,event.acceleration.z);
			fprintf(accF, "%lld; %f; %f; %f; %f;\n", event.timestamp, ((double)(event.timestamp-lastAccTime))/1000000000.0, event.acceleration.azimuth,event.acceleration.pitch,event.acceleration.roll);
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
			fprintf(gyrF, "%lld; %f; %f; %f; %f;\n", event.timestamp, ((double)(event.timestamp-lastGyrTime))/1000000000.0, event.acceleration.azimuth,event.acceleration.pitch,event.acceleration.roll);
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

static int get_mag_events(int fd, int events, void* data){
	ASensorEvent event;
	while(ASensorEventQueue_getEvents(magQueue, &event, 1) >0 && (magSampleCount < SAMPLE_LIMIT)){
		if(event.type == ASENSOR_TYPE_MAGNETIC_FIELD){
			//LOGI("[%d][%f][%f][%f][%f]", gyrSampleCount, ((double)(event.timestamp-lastGyrTime))/1000000000.0, event.acceleration.x,event.acceleration.y,event.acceleration.z);
			fprintf(magF, "%lld; %f; %f; %f; %f;\n", event.timestamp, ((double)(event.timestamp-lastMagTime))/1000000000.0, event.magnetic.x,event.magnetic.y,event.magnetic.z);
			lastMagTime = event.timestamp;
			magSampleCount++;
		}

		if(magSampleCount == SAMPLE_LIMIT){
			LOGI("MAG SAMPLING FINISHED");
			fclose(magF);
		}

	}
	return 1;
}

static int get_bar_events(int fd, int events, void* data){
	ASensorEvent event;
	while(ASensorEventQueue_getEvents(barQueue, &event, 1) >0 && (barSampleCount < BAR_LIMIT)){
		if(event.type == 6){
			//LOGI("[%d][%f][%f][%f][%f]", gyrSampleCount, ((double)(event.timestamp-lastGyrTime))/1000000000.0, event.acceleration.x,event.acceleration.y,event.acceleration.z);
			fprintf(barF, "%lld; %f; %f;\n", event.timestamp, ((double)(event.timestamp-lastBarTime))/1000000000.0, event.pressure);
			lastBarTime = event.timestamp;
			barSampleCount++;
		}

		if(barSampleCount == BAR_LIMIT){
			LOGI("BAR SAMPLING FINISHED");
			fclose(barF);
		}

	}
	return 1;
}
