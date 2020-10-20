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

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    JNIEnv * HAZELCAST_API GetJniEnv(JavaVM *jvm);

    class HAZELCAST_API IMap {
    public:
        IMap(jobject object, const char *name, JavaVM *vm,
             std::shared_ptr<client::serialization::pimpl::SerializationService> serializationService);

        virtual ~IMap();

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

        jbyteArray convertDataToJByteArray(client::serialization::pimpl::Data &data);
    };


    class HAZELCAST_API HazelcastInstance {
    public:
        HazelcastInstance(jobject object, JavaVM *vm, bool isMember);

        virtual ~HazelcastInstance();

        std::string getName() const;

        std::shared_ptr<IMap> getMap(const std::string &name);

        void shutdown();

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
        explicit Hazelcast(const std::string &hazelcastJarPath) ;

        std::shared_ptr<HazelcastInstance> newHazelcastInstance() ;

        std::shared_ptr<HazelcastInstance> newHazelcastInstance(const std::string& xml) ;

        std::shared_ptr<HazelcastInstance> newHazelcastClient();

        std::shared_ptr<HazelcastInstance> newHazelcastClient(const std::string& xml);

        void shutdownAll();

        void detach_current_thread();

        ~Hazelcast();

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
