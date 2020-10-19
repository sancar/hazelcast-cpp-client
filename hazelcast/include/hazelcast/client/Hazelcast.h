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

    class HAZELCAST_API IMap {
    public:
        IMap(jobject object, const char *name, JNIEnv *env,
             std::shared_ptr<client::serialization::pimpl::SerializationService> serializationService)
                : env(env), serializationService(serializationService), object(object) {
            cls = env->FindClass(name);
            if (cls == nullptr) {
                throw std::exception();
            }

            putId = env->GetMethodID(cls, "putForC", "([B[B)[B");
            if (putId == nullptr) {
                throw std::exception();
            }

            getId = env->GetMethodID(cls, "getForC", "([B)[B");
            if (getId == nullptr) {
                throw std::exception();
            }
        }

        ~IMap() {
            env->DeleteLocalRef(object);
            env->DeleteLocalRef(cls);
        }

        template<typename K, typename V>
        boost::optional<V> get(const K &key) {
            client::serialization::pimpl::Data data = toData(key);
            jbyteArray keyByteArray = convertDataToJByteArray(data);
            jbyteArray response = (jbyteArray) env->CallObjectMethod(object, getId, keyByteArray);
            env->DeleteLocalRef(keyByteArray);

            return convertJByteArrayToObject<V>(response);
        }

        template<typename K, typename V, typename R=V>
        boost::optional<R> put(const K &key, const V &value) {
            client::serialization::pimpl::Data keyData = toData(key);
            client::serialization::pimpl::Data valueData = toData(value);
            jbyteArray keyByteArray = convertDataToJByteArray(keyData);
            jbyteArray valueByteArray = convertDataToJByteArray(valueData);
            jbyteArray response = (jbyteArray) env->CallObjectMethod(object, putId, keyByteArray, valueByteArray);
            env->DeleteLocalRef(keyByteArray);
            env->DeleteLocalRef(valueByteArray);

            return convertJByteArrayToObject<V>(response);
        }

    private:
        JNIEnv *env;
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

            jsize arrayLength = env->GetArrayLength(value);
            jboolean iscopy = JNI_TRUE;
            jbyte *arrayElements = env->GetByteArrayElements(value, &iscopy);
            byte *bytes = reinterpret_cast<byte *>(arrayElements);


            std::vector<byte> responseData;
            auto iter = responseData.begin();
            responseData.insert(iter, bytes, bytes + arrayLength);

            hazelcast::client::serialization::pimpl::Data data(responseData);
            boost::optional<V> responseObject = toObject<V>(responseData);
            jint m = 0;
            env->ReleaseByteArrayElements(value, arrayElements, m);

            return responseObject;
        }

        jbyteArray convertDataToJByteArray(client::serialization::pimpl::Data &data) {
            std::vector<byte> byteArray = data.toByteArray();
            jbyteArray keyByteArray = env->NewByteArray(byteArray.size());
            env->SetByteArrayRegion(keyByteArray, 0, (jsize) byteArray.size(), (jbyte *) byteArray.data());
            return keyByteArray;
        }

    };


    class HAZELCAST_API HazelcastInstance {
    public:
        HazelcastInstance(jobject object, JNIEnv *env, bool isMember)
                : env(env), serializationService(
                new client::serialization::pimpl::SerializationService(serializationConfig)) {
            this->isMember = isMember;
            this->object = object;
            this->env = env;
            cls = env->FindClass("com/hazelcast/core/HazelcastInstance");
            if (cls == nullptr) {
                throw std::exception();
            }

            getNameId = env->GetMethodID(cls, "getName", "()Ljava/lang/String;");
            if (getNameId == nullptr) {
                throw std::exception();
            }
            shutdownId = env->GetMethodID(cls, "shutdown", "()V");
            if (shutdownId == nullptr) {
                throw std::exception();
            }
            getMapId = env->GetMethodID(cls, "getMap", "(Ljava/lang/String;)Lcom/hazelcast/map/IMap;");
            if (getMapId == nullptr) {
                throw std::exception();
            }
        };

        ~HazelcastInstance() {
            env->DeleteLocalRef(object);
            env->DeleteLocalRef(cls);
        }

        std::string getName() const {
            jstring obj = (jstring) env->CallObjectMethod(object, getNameId);
            jboolean iscopy = JNI_TRUE;
            const char *chars = env->GetStringUTFChars(obj, &iscopy);
            std::string name(chars);
            env->ReleaseStringUTFChars(obj, chars);
            return name;
        }

        std::shared_ptr<IMap> getMap(const std::string &name) {
            jstring mapName = env->NewStringUTF(name.c_str());
            jobject obj = env->CallObjectMethod(object, getMapId, mapName);

            if (isMember) {
                return std::shared_ptr<IMap>(
                        new IMap(obj, "com/hazelcast/map/impl/proxy/MapProxyImpl", env, serializationService));
            } else {
                return std::shared_ptr<IMap>(
                        new IMap(obj, "com/hazelcast/client/impl/proxy/ClientMapProxy", env, serializationService));
            }
        }

        void shutdown() {
            env->CallObjectMethod(object, shutdownId);
        }

    private:
        JNIEnv *env;
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
            optionString = "-Djava.class.path=" + hazelcastJarPath;
            options[0].optionString = (char *) optionString.c_str();
            vm_args.options = options;
            vm_args.nOptions = 1;
            vm_args.ignoreUnrecognized = JNI_TRUE;
            JavaVM *javaVM = NULL;
            JNIEnv *jniEnv = NULL;
            jint res = JNI_CreateJavaVM(&javaVM, (void **) &jniEnv, &vm_args);
            if (res != JNI_OK) {
                throw std::exception();
            }
            this->env = jniEnv;
            this->vm = javaVM;

            cls = env->FindClass("com/hazelcast/core/Hazelcast");
            if (cls == nullptr) {
                throw std::exception();
            }
            clientCls = env->FindClass("com/hazelcast/client/HazelcastClient");
            if (clientCls == nullptr) {
                throw std::exception();
            }
            newHazelcastInstanceId = env->GetStaticMethodID(cls, "newHazelcastInstance",
                                                            "()Lcom/hazelcast/core/HazelcastInstance;");
            if (newHazelcastInstanceId == nullptr) {
                throw std::exception();
            }
            newHazelcastClientId = env->GetStaticMethodID(clientCls, "newHazelcastClient",
                                                          "()Lcom/hazelcast/core/HazelcastInstance;");
            if (newHazelcastClientId == nullptr) {
                throw std::exception();
            }
            shutdownAllId = env->GetStaticMethodID(cls, "shutdownAll", "()V");
            if (shutdownAllId == nullptr) {
                throw std::exception();
            }
        }

        std::shared_ptr<HazelcastInstance> newHazelcastInstance() {
            jobject object = env->CallStaticObjectMethod(cls, newHazelcastInstanceId);
            return std::shared_ptr<HazelcastInstance>(new HazelcastInstance(object, env, true));
        }


        std::shared_ptr<HazelcastInstance> newHazelcastClient() {
            jobject obj = env->CallStaticObjectMethod(clientCls, newHazelcastClientId);
            return std::shared_ptr<HazelcastInstance>(new HazelcastInstance(obj, env, false));
        }


        void shutdownAll() {
            env->CallStaticVoidMethod(cls, shutdownAllId);
        }

        ~Hazelcast() {
            env->DeleteLocalRef(cls);
            env->DeleteLocalRef(clientCls);
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
        JNIEnv *env;
        JavaVM *vm;
        jclass cls;
        jclass clientCls;
        jmethodID newHazelcastInstanceId;
        jmethodID newHazelcastClientId;
        jmethodID shutdownAllId;
    };


}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
