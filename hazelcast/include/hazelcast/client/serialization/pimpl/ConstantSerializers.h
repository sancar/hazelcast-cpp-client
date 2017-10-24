/*
 * Copyright (c) 2008-2017, Hazelcast, Inc. All Rights Reserved.
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

#ifndef HAZELCAST_CLIENT_SERIALIZATION_PIMPL_CANSTANTSERIALIZERS_H_
#define HAZELCAST_CLIENT_SERIALIZATION_PIMPL_CANSTANTSERIALIZERS_H_

#include <memory>
#include <stdint.h>
#include <boost/shared_ptr.hpp>

#include "hazelcast/util/HazelcastDll.h"
#include "hazelcast/client/serialization/Serializer.h"

namespace hazelcast {
    namespace client {

        class SerializationConfig;

        namespace serialization {
            class IdentifiedDataSerializable;

            class ObjectDataOutput;

            class ObjectDataInput;

            class IdentifiedDataSerializable;

            class DataSerializableFactory;

            namespace pimpl {
                class ByteSerializer : public Serializer<byte> {
                public:
                    virtual int32_t getHazelcastTypeId() const;

                    virtual void write(ObjectDataOutput &out, const byte &object);

                    virtual void read(ObjectDataInput &in, byte &object);

                    virtual void *create();
                };

                class BooleanSerializer : public Serializer<bool> {
                public:
                    virtual int32_t getHazelcastTypeId() const;

                    virtual void write(ObjectDataOutput &out, const bool &object);

                    virtual void *create();

                    virtual void read(ObjectDataInput &in, bool &object);
                };

                class IntegerSerializer : public Serializer<int32_t> {
                public:
                    virtual int32_t getHazelcastTypeId() const;

                    virtual void write(ObjectDataOutput &out, const int32_t &object);

                    virtual void read(ObjectDataInput &in, int32_t &object);

                    virtual void *create();
                };

            }
        }
    }
}

#endif //HAZELCAST_CLIENT_SERIALIZATION_PIMPL_CANSTANTSERIALIZERS_H_

