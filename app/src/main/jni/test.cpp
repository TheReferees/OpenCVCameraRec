#include <jni.h>
#include <iostream>

extern "C" {
	JNIEXPORT jint JNICALL Java_com_thereferees_opencvcamerarec_MainActivity_testing ( JNIEnv* env, jobject, jint num){
		jint returned;
		returned = (jint) num * 5;
		return returned;
	}
}
