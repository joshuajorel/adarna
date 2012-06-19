#include <android/log.h>
#include <stdlib.h>
#include <jni.h>
#include <android/sensor.h>
#include <android/looper.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>

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
int64_t lastGpsTime = 0;
FILE* accF;
FILE* gyrF;
FILE* magF;
FILE* barF;

int accSampleCount = 0;
int gyrSampleCount = 0;
int magSampleCount = 0;
int barSampleCount = 0;
int accFlag = 0;
int gyrFlag = 0;
int magFlag = 0;
int barFlag = 0;
int stopFlag = 0;

double latC = 0;
double lonC = 0;
double altC = 0;

void *acc_data;
void *gyr_data;
void *mag_data;
void *bar_data;

const ASensor *accSensor;
const ASensor *gyrSensor;
const ASensor *magSensor;
const ASensor *barSensor;

ASensorEventQueue* accQueue;
ASensorEventQueue* gyrQueue;
ASensorEventQueue* magQueue;
ASensorEventQueue* barQueue;

static int get_acc_events(int fd, int events, void* data);
static int get_gyr_events(int fd, int events, void* data);
static int get_mag_events(int fd, int events, void* data);
static int get_bar_events(int fd, int events, void* data);

ASensorManager* accSensorManager;
ASensorManager* gyrSensorManager;
ASensorManager* magSensorManager;
ASensorManager* barSensorManager;

ALooper* accLooper;
ALooper* gyrLooper;
ALooper* magLooper;
ALooper* barLooper;

static pthread_mutex_t mutex;

void *input_gps();

pthread_t thread;

char *message = "Whatevah, whatevah, I do what I want.";

JNIEXPORT jstring JNICALL Java_com_Threading_ThreadActivity_goThread
  (JNIEnv *env, jclass class){
	LOGI("Sensor Data Start");
	return (*env)->NewStringUTF(env, "Acquiring Sensor Data");
}

JNIEXPORT void JNICALL Java_com_Threading_ThreadActivity_stop
  (JNIEnv *env, jclass class){
	stopFlag = 1;
}

JNIEXPORT void JNICALL Java_com_Threading_ThreadActivity_setGps
 (JNIEnv *env, jclass class, jdouble lat, jdouble lon, jdouble alt){
	pthread_mutex_lock(&mutex);
	//LOGI("GPS Locked on Mother Fucker");
	latC = lat;
	lonC = lon;
	altC = alt;
	pthread_mutex_unlock(&mutex);
}

JNIEXPORT void JNICALL Java_com_Threading_ThreadActivity_threader
  (JNIEnv *env, jclass class){
	pthread_create(&thread, NULL, input_gps, NULL);
	ASensorEvent accEvent;
	ASensorEvent gyrEvent;
	ASensorEvent magEvent;
	ASensorEvent barEvent;

	acc_data = malloc(1000);
	gyr_data = malloc(1000);
	mag_data = malloc(1000);
	bar_data = malloc(1000);


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

	accLooper = ALooper_forThread();
	gyrLooper = ALooper_forThread();
	magLooper = ALooper_forThread();
	barLooper = ALooper_forThread();

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

	/*while(!accFlag || !gyrFlag || !magFlag || !barFlag){

	}
JNIEXPORT void JNICALL Java_com_Threading_ThreadActivity_setGps
 (JNIEnv *env, jclass class, jlong timeM){

	pthread_create(&thread, NULL, print_message_function, (long*) timeM);


}
	return 1;*/
}

static int get_acc_events(int fd, int events, void* data){
	ASensorEvent event;
	while(ASensorEventQueue_getEvents(accQueue, &event, 1) >0 && !stopFlag){
		//(accSampleCount < SAMPLE_LIMIT)
		if(event.type == ASENSOR_TYPE_ACCELEROMETER){
			//LOGI("[%d][%f][%f][%f][%f]", accSampleCount, ((double)(event.timestamp-lastAccTime))/1000000000.0, event.acceleration.x,event.acceleration.y,event.acceleration.z);
			fprintf(accF, "%lld; %f; %f; %f; %f;\n", event.timestamp, ((double)(event.timestamp-lastAccTime))/1000000000.0, event.acceleration.azimuth,event.acceleration.pitch,event.acceleration.roll);
			fclose(accF);
			lastAccTime = event.timestamp;
			accSampleCount++;
		}
	}

	if(stopFlag){
		LOGI("ACC SAMPLING FINISHED");
		accFlag = 1;
		ASensorEventQueue_disableSensor(accQueue, accSensor);
		LOGI("Disabled Acc Sensor");
		ASensorManager_destroyEventQueue(accSensorManager, accQueue);
		LOGI("Freed acc manager");
//			ALooper_release(accLooper);
//			LOGI("Freed acc looper");
		free(acc_data);
		LOGI("Freed acc data");
	}
	return 1;
}

static int get_gyr_events(int fd, int events, void* data){
	ASensorEvent event;
	while(ASensorEventQueue_getEvents(gyrQueue, &event, 1) >0 && !stopFlag){
		if(event.type == ASENSOR_TYPE_GYROSCOPE){
			//LOGI("[%d][%f][%f][%f][%f]", gyrSampleCount, ((double)(event.timestamp-lastGyrTime))/1000000000.0, event.acceleration.x,event.acceleration.y,event.acceleration.z);
			fprintf(gyrF, "%lld; %f; %f; %f; %f;\n", event.timestamp, ((double)(event.timestamp-lastGyrTime))/1000000000.0, event.acceleration.azimuth,event.acceleration.pitch,event.acceleration.roll);
			fclose(gyrF);
			lastGyrTime = event.timestamp;
			gyrSampleCount++;
		}
	}

	if(stopFlag){
		LOGI("GYR SAMPLING FINISHED");
		gyrFlag = 1;
		ASensorEventQueue_disableSensor(gyrQueue, gyrSensor);
		LOGI("Disabled gyr Sensor");
		ASensorManager_destroyEventQueue(gyrSensorManager, gyrQueue);
		LOGI("Freed gyr manager");
	//	ALooper_release(gyrLooper);
	//	LOGI("Freed gyr looper");
		free(gyr_data);
		LOGI("Freed gyr data");
	}
	return 1;
}

static int get_mag_events(int fd, int events, void* data){
	ASensorEvent event;
	while(ASensorEventQueue_getEvents(magQueue, &event, 1) >0 && !stopFlag){
		if(event.type == ASENSOR_TYPE_MAGNETIC_FIELD){
			//LOGI("[%d][%f][%f][%f][%f]", gyrSampleCount, ((double)(event.timestamp-lastGyrTime))/1000000000.0, event.acceleration.x,event.acceleration.y,event.acceleration.z);
			fprintf(magF, "%lld; %f; %f; %f; %f;\n", event.timestamp, ((double)(event.timestamp-lastMagTime))/1000000000.0, event.magnetic.x,event.magnetic.y,event.magnetic.z);
			fclose(magF);
			lastMagTime = event.timestamp;
			magSampleCount++;
		}
	}

	if(stopFlag){
		LOGI("MAG SAMPLING FINISHED");
		magFlag = 1;
		ASensorEventQueue_disableSensor(magQueue, magSensor);
		LOGI("Disabled mag Sensor");
		ASensorManager_destroyEventQueue(magSensorManager, magQueue);
		LOGI("Freed mag manager");
//		ALooper_release(magLooper);
//		LOGI("Freed mag looper");
		free(mag_data);
		LOGI("Freed mag data");
	}

	return 1;
}

static int get_bar_events(int fd, int events, void* data){
	ASensorEvent event;
	while(ASensorEventQueue_getEvents(barQueue, &event, 1) >0 && !stopFlag){
		if(event.type == 6){
			//LOGI("[%d][%f][%f][%f][%f]", gyrSampleCount, ((double)(event.timestamp-lastGyrTime))/1000000000.0, event.acceleration.x,event.acceleration.y,event.acceleration.z);
			fprintf(barF, "%lld; %f; %f;\n", event.timestamp, ((double)(event.timestamp-lastBarTime))/1000000000.0, event.pressure);
			lastBarTime = event.timestamp;
			barSampleCount++;
		}

		}

	if(stopFlag){
		LOGI("BAR SAMPLING FINISHED");
		barFlag = 1;
		ASensorEventQueue_disableSensor(barQueue, barSensor);
		LOGI("Disabled bar Sensor");
		ASensorManager_destroyEventQueue(barSensorManager, barQueue);
		LOGI("Freed bar manager");
//		ALooper_release(barLooper);
//		LOGI("Freed bar looper");
		free(bar_data);
		LOGI("Freed bar data");
	}
	return 1;
}

void *input_gps()
{
	int ctr = 0;
	FILE *gpsF;
	gpsF= fopen("/sdcard/gps-samp.csv", "w");

	while (!stopFlag){
		pthread_mutex_lock(&mutex);
		fprintf(gpsF, "%lf; %lf; %lf;\n", latC, lonC, altC);

		pthread_mutex_unlock(&mutex);
		sleep(1);
	}
	LOGI("GPS STOP");
	fclose(gpsF);
}
