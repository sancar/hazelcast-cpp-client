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

#include <hazelcast/client/serialization/pimpl/SerializationConstants.h>
#include "hazelcast/client/serialization/pimpl/ConstantSerializers.h"
#include "hazelcast/client/serialization/ObjectDataInput.h"
#include "hazelcast/client/serialization/ObjectDataOutput.h"

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {

                int32_t IntegerSerializer::getHazelcastTypeId() const {
                    return SerializationConstants::CONSTANT_TYPE_INTEGER;
                }

                void IntegerSerializer::write(ObjectDataOutput &out, const int32_t &object) {
                    out.writeInt(object);
                }

                void IntegerSerializer::read(ObjectDataInput &in, int32_t &object) {
                    object = in.readInt();
                }

                void *IntegerSerializer::create() {
                    return new int32_t;
                }

                int32_t ByteSerializer::getHazelcastTypeId() const {
                    return SerializationConstants::CONSTANT_TYPE_BYTE;
                }

                void ByteSerializer::write(ObjectDataOutput &out, const byte &object) {
                    out.writeByte(object);
                }

                void ByteSerializer::read(ObjectDataInput &in, byte &object) {
                    object = in.readByte();
                }

                void *ByteSerializer::create() {
                    return new byte;
                }

                int32_t BooleanSerializer::getHazelcastTypeId() const {
                    return SerializationConstants::CONSTANT_TYPE_BOOLEAN;
                }

                void BooleanSerializer::write(ObjectDataOutput &out, const bool &object) {
                    out.writeBoolean(object);
                }

                void BooleanSerializer::read(ObjectDataInput &in, bool &object) {
                    object = in.readBoolean();
                }

                void *BooleanSerializer::create() {
                    return new bool;
                }
            }
        }
    }
}


