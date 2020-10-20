/*
 * Copyright (c) 2008-2020, Hazelcast, Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#pragma once

#include "hazelcast/client/serialization/serialization.h"
#include "hazelcast/client/SerializationConfig.h"
#include <jni.h>
#include <stdlib.h>

#include <utility>

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {


    JNIEnv *GetJniEnv(JavaVM *jvm) {
        JNIEnv *env;

        jint attachRes = jvm->AttachCurrentThread(reinterpret_cast<void **>(&env), NULL);

        if (attachRes != JNI_OK) {
            throw std::runtime_error("attach failed");
        }

        return env;
    }

    class HAZELCAST_API IMap {
    public:
        IMap(jobject object, const char *name, JavaVM *vm,
             std::shared_ptr<client::serialization::pimpl::SerializationService> serializationService)
                : vm(vm), serializationService(serializationService), object(object) {
            JNIEnv *pEnv = GetJniEnv(vm);
            cls = pEnv->FindClass(name);
            if (cls == nullptr) {
                throw std::exception();
            }

            putId = pEnv->GetMethodID(cls, "putForC", "([B[B)[B");
            if (putId == nullptr) {
                throw std::exception();
            }

            getId = pEnv->GetMethodID(cls, "getForC", "([B)[B");
            if (getId == nullptr) {
                throw std::exception();
            }
        }

        ~IMap() {
            JNIEnv *pEnv = GetJniEnv(vm);
            pEnv->DeleteLocalRef(object);
            pEnv->DeleteLocalRef(cls);
        }

        template<typename K, typename V>
        boost::optional<V> get(const K &key) {
            client::serialization::pimpl::Data data = toData(key);
            jbyteArray keyByteArray = convertDataToJByteArray(data);
            JNIEnv *pEnv = GetJniEnv(vm);
            jbyteArray response = (jbyteArray) pEnv->CallObjectMethod(object, getId, keyByteArray);
            pEnv->DeleteLocalRef(keyByteArray);

            return convertJByteArrayToObject<V>(response);
        }

        template<typename K, typename V, typename R=V>
        boost::optional<R> put(const K &key, const V &value) {
            client::serialization::pimpl::Data keyData = toData(key);
            client::serialization::pimpl::Data valueData = toData(value);
            jbyteArray keyByteArray = convertDataToJByteArray(keyData);
            jbyteArray valueByteArray = convertDataToJByteArray(valueData);
            JNIEnv *pEnv = GetJniEnv(vm);
            jbyteArray response = (jbyteArray) pEnv->CallObjectMethod(object, putId, keyByteArray, valueByteArray);
            pEnv->DeleteLocalRef(keyByteArray);
            pEnv->DeleteLocalRef(valueByteArray);

            return convertJByteArrayToObject<V>(response);
        }

    private:
        JavaVM* vm;
        std::shared_ptr<client::serialization::pimpl::SerializationService> serializationService;
        jclass cls;
        jobject object;
        jmethodID putId;
        jmethodID getId;

        template<typename T>
        client::serialization::pimpl::Data toData(const T &obj) {
            return serializationService->template toData<T>(&obj);
        }

        template<typename T>
        inline boost::optional<T> toObject(const client::serialization::pimpl::Data &data) {
            return serializationService->template toObject<T>(data);
        }

        template<typename V>
        boost::optional<V> convertJByteArrayToObject(jbyteArray value) {
            if (value == nullptr) {
                return boost::none;
            }

            JNIEnv *pEnv = GetJniEnv(vm);
            jsize arrayLength = pEnv->GetArrayLength(value);
            jboolean iscopy = JNI_TRUE;
            jbyte *arrayElements = pEnv->GetByteArrayElements(value, &iscopy);
            byte *bytes = reinterpret_cast<byte *>(arrayElements);


            std::vector<byte> responseData;
            auto iter = responseData.begin();
            responseData.insert(iter, bytes, bytes + arrayLength);

            hazelcast::client::serialization::pimpl::Data data(responseData);
            boost::optional<V> responseObject = toObject<V>(responseData);
            jint m = 0;
            pEnv->ReleaseByteArrayElements(value, arrayElements, m);

            return responseObject;
        }

        jbyteArray convertDataToJByteArray(client::serialization::pimpl::Data &data) {
            std::vector<byte> byteArray = data.toByteArray();
            JNIEnv *pEnv = GetJniEnv(vm);
            jbyteArray keyByteArray = pEnv->NewByteArray(byteArray.size());
            pEnv->SetByteArrayRegion(keyByteArray, 0, (jsize) byteArray.size(), (jbyte *) byteArray.data());
            return keyByteArray;
        }

    };


    class HAZELCAST_API HazelcastInstance {
    public:
        HazelcastInstance(jobject object, JavaVM *vm, bool isMember)
                : vm(vm), serializationService(
                new client::serialization::pimpl::SerializationService(serializationConfig)) {
            this->isMember = isMember;
            this->object = object;
            JNIEnv *pEnv = GetJniEnv(vm);
            cls = pEnv->FindClass("com/hazelcast/core/HazelcastInstance");
            if (cls == nullptr) {
                throw std::exception();
            }

            getNameId = pEnv->GetMethodID(cls, "getName", "()Ljava/lang/String;");
            if (getNameId == nullptr) {
                throw std::exception();
            }
            shutdownId = pEnv->GetMethodID(cls, "shutdown", "()V");
            if (shutdownId == nullptr) {
                throw std::exception();
            }
            getMapId = pEnv->GetMethodID(cls, "getMap", "(Ljava/lang/String;)Lcom/hazelcast/map/IMap;");
            if (getMapId == nullptr) {
                throw std::exception();
            }
        };

        ~HazelcastInstance() {
            JNIEnv *pEnv = GetJniEnv(vm);
            pEnv->DeleteLocalRef(object);
            pEnv->DeleteLocalRef(cls);
        }

        std::string getName() const {
            JNIEnv *pEnv = GetJniEnv(vm);
            jstring obj = (jstring) pEnv->CallObjectMethod(object, getNameId);
            jboolean iscopy = JNI_TRUE;
            const char *chars = pEnv->GetStringUTFChars(obj, &iscopy);
            std::string name(chars);
            pEnv->ReleaseStringUTFChars(obj, chars);
            return name;
        }

        std::shared_ptr<IMap> getMap(const std::string &name) {
            JNIEnv *pEnv = GetJniEnv(vm);
            jstring mapName = pEnv->NewStringUTF(name.c_str());
            jobject obj = pEnv->CallObjectMethod(object, getMapId, mapName);
            pEnv->DeleteLocalRef(mapName);
            if (isMember) {
                return std::shared_ptr<IMap>(
                        new IMap(obj, "com/hazelcast/map/impl/proxy/MapProxyImpl", vm, serializationService));
            } else {
                return std::shared_ptr<IMap>(
                        new IMap(obj, "com/hazelcast/client/impl/proxy/ClientMapProxy", vm, serializationService));
            }
        }

        void shutdown() {
            GetJniEnv(vm)->CallObjectMethod(object, shutdownId);
        }

    private:
        JavaVM* vm;
        bool isMember;
        client::SerializationConfig serializationConfig;
        std::shared_ptr<client::serialization::pimpl::SerializationService> serializationService;
        jclass cls;
        jobject object;
        jmethodID getNameId;
        jmethodID shutdownId;
        jmethodID getMapId;
    };

    class HAZELCAST_API Hazelcast {
    public:
        explicit Hazelcast(const std::string &hazelcastJarPath) {
            vm_args.version = JNI_VERSION_1_8;
            optionString.append("-Djava.class.path=").append(hazelcastJarPath);
            options[0].optionString = (char *) optionString.c_str();
            vm_args.options = options;
            vm_args.nOptions = 1;
            vm_args.ignoreUnrecognized = JNI_TRUE;
            if (JNI_GetDefaultJavaVMInitArgs(&vm_args) != JNI_OK) {
                throw std::exception();
            }
            JavaVM *javaVM = NULL;
            JNIEnv *jniEnv = NULL;
            jint res = JNI_CreateJavaVM(&javaVM, (void **) &jniEnv, &vm_args);
            if (res != JNI_OK) {
                throw std::exception();
            }
            this->vm = javaVM;
            cls = jniEnv->FindClass("com/hazelcast/core/Hazelcast");
            if (cls == nullptr) {
                throw std::exception();
            }
            clientCls = jniEnv->FindClass("com/hazelcast/client/HazelcastClient");
            if (clientCls == nullptr) {
                throw std::exception();
            }
            newHazelcastInstanceId = jniEnv->GetStaticMethodID(cls, "newHazelcastInstance",
                                                            "()Lcom/hazelcast/core/HazelcastInstance;");
            if (newHazelcastInstanceId == nullptr) {
                throw std::exception();
            }
            newHazelcastInstanceWithConfigId = jniEnv->GetStaticMethodID(cls, "newHazelcastInstanceWithConfig",
                                                               "(Ljava/lang/String;)Lcom/hazelcast/core/HazelcastInstance;");
            if (newHazelcastInstanceWithConfigId == nullptr) {
                throw std::exception();
            }
            newHazelcastClientId = jniEnv->GetStaticMethodID(clientCls, "newHazelcastClient",
                                                          "()Lcom/hazelcast/core/HazelcastInstance;");
            if (newHazelcastClientId == nullptr) {
                throw std::exception();
            }
            newHazelcastClientWithConfigId = jniEnv->GetStaticMethodID(clientCls, "newHazelcastClientWithConfig",
                                                             "(Ljava/lang/String;)Lcom/hazelcast/core/HazelcastInstance;");
            if (newHazelcastClientWithConfigId == nullptr) {
                throw std::exception();
            }
            shutdownAllId = jniEnv->GetStaticMethodID(cls, "shutdownAll", "()V");
            if (shutdownAllId == nullptr) {
                throw std::exception();
            }
        }

        std::shared_ptr<HazelcastInstance> newHazelcastInstance() {
            jobject object = GetJniEnv(vm)->CallStaticObjectMethod(cls, newHazelcastInstanceId);
            return std::shared_ptr<HazelcastInstance>(new HazelcastInstance(object, vm, true));
        }

        std::shared_ptr<HazelcastInstance> newHazelcastInstance(const std::string& xml) {
            JNIEnv *pEnv = GetJniEnv(vm);
            jstring xmlStr = pEnv->NewStringUTF(xml.c_str());
            jobject object = pEnv->CallStaticObjectMethod(cls, newHazelcastInstanceWithConfigId, xmlStr);
            if(pEnv->ExceptionOccurred()) {
                pEnv->ExceptionDescribe();
            }
            pEnv->DeleteLocalRef(xmlStr);
            return std::shared_ptr<HazelcastInstance>(new HazelcastInstance(object, vm, true));
        }


        std::shared_ptr<HazelcastInstance> newHazelcastClient() {
            jobject obj = GetJniEnv(vm)->CallStaticObjectMethod(clientCls, newHazelcastClientId);
            return std::shared_ptr<HazelcastInstance>(new HazelcastInstance(obj, vm, false));
        }

        std::shared_ptr<HazelcastInstance> newHazelcastClient(const std::string& xml) {
            JNIEnv *pEnv = GetJniEnv(vm);
            jstring xmlStr = pEnv->NewStringUTF(xml.c_str());
            jobject obj = pEnv->CallStaticObjectMethod(clientCls, newHazelcastClientWithConfigId, xmlStr);
            pEnv->DeleteLocalRef(xmlStr);
            return std::shared_ptr<HazelcastInstance>(new HazelcastInstance(obj, vm, false));
        }

        void shutdownAll() {
            GetJniEnv(vm)->CallStaticVoidMethod(cls, shutdownAllId);
        }

        ~Hazelcast() {
            JNIEnv *pEnv = GetJniEnv(vm);
            pEnv->DeleteLocalRef(cls);
            pEnv->DeleteLocalRef(clientCls);
            jint ok = vm->DetachCurrentThread();
            if (ok != JNI_OK) {
                std::cout << "Failed to DetachCurrentThread" << std::endl;
            }
            ok = vm->DestroyJavaVM();
            if (ok != JNI_OK) {
                std::cout << "Failed to DestroyJavaVM" << std::endl;
            }

        }

    private:
        JavaVMInitArgs vm_args;
        JavaVMOption options[1];
        std::string optionString;
        JavaVM *vm;
        jclass cls;
        jclass clientCls;
        jmethodID newHazelcastInstanceId;
        jmethodID newHazelcastInstanceWithConfigId;
        jmethodID newHazelcastClientId;
        jmethodID newHazelcastClientWithConfigId;
        jmethodID shutdownAllId;
    };


}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
