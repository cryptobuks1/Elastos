/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class com_breadwallet_core_BRCoreKey */

#ifndef _Included_com_breadwallet_core_BRCoreKey
#define _Included_com_breadwallet_core_BRCoreKey
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     com_breadwallet_core_BRCoreKey
 * Method:    getSecret
 * Signature: ()[B
 */
JNIEXPORT jbyteArray JNICALL Java_com_breadwallet_core_BRCoreKey_getSecret
  (JNIEnv *, jobject);

/*
 * Class:     com_breadwallet_core_BRCoreKey
 * Method:    getPubKey
 * Signature: ()[B
 */
JNIEXPORT jbyteArray JNICALL Java_com_breadwallet_core_BRCoreKey_getPubKey
  (JNIEnv *, jobject);

/*
 * Class:     com_breadwallet_core_BRCoreKey
 * Method:    getCompressed
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_com_breadwallet_core_BRCoreKey_getCompressed
  (JNIEnv *, jobject);

/*
 * Class:     com_breadwallet_core_BRCoreKey
 * Method:    createJniCoreKey
 * Signature: ()J
 */
JNIEXPORT jlong JNICALL Java_com_breadwallet_core_BRCoreKey_createJniCoreKey
  (JNIEnv *, jclass);

/*
 * Class:     com_breadwallet_core_BRCoreKey
 * Method:    setPrivKey
 * Signature: ([B)Z
 */
JNIEXPORT jboolean JNICALL Java_com_breadwallet_core_BRCoreKey_setPrivKey
  (JNIEnv *, jobject, jbyteArray);

/*
 * Class:     com_breadwallet_core_BRCoreKey
 * Method:    setSecret
 * Signature: ([B)V
 */
JNIEXPORT void JNICALL Java_com_breadwallet_core_BRCoreKey_setSecret
  (JNIEnv *, jobject, jbyteArray);

/*
 * Class:     com_breadwallet_core_BRCoreKey
 * Method:    compactSign
 * Signature: ([B)[B
 */
JNIEXPORT jbyteArray JNICALL Java_com_breadwallet_core_BRCoreKey_compactSign
  (JNIEnv *, jobject, jbyteArray);

/*
 * Class:     com_breadwallet_core_BRCoreKey
 * Method:    encryptNative
 * Signature: ([B[B)[B
 */
JNIEXPORT jbyteArray JNICALL Java_com_breadwallet_core_BRCoreKey_encryptNative
  (JNIEnv *, jobject, jbyteArray, jbyteArray);

/*
 * Class:     com_breadwallet_core_BRCoreKey
 * Method:    decryptNative
 * Signature: ([B[B)[B
 */
JNIEXPORT jbyteArray JNICALL Java_com_breadwallet_core_BRCoreKey_decryptNative
  (JNIEnv *, jobject, jbyteArray, jbyteArray);

/*
 * Class:     com_breadwallet_core_BRCoreKey
 * Method:    address
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_breadwallet_core_BRCoreKey_address
  (JNIEnv *, jobject);

#ifdef __cplusplus
}
#endif
#endif
