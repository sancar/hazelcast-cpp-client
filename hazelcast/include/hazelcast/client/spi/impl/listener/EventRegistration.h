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
//
// Created by ihsan demir on 10/11/15.

#ifndef HAZELCAST_CLIENT_SPI_IMPL_LISTENER_EVENTREGISTRATION_H_
#define HAZELCAST_CLIENT_SPI_IMPL_LISTENER_EVENTREGISTRATION_H_

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

#include <string>
#include <stdint.h>
#include <memory>

#include "hazelcast/util/HazelcastDll.h"
#include "hazelcast/client/Address.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            class ClientMessage;
            namespace codec {
                class IAddListenerCodec;
            }
        }
        namespace spi {
            namespace impl {
                namespace listener {
					class HAZELCAST_API EventRegistration {

                    public:
                        EventRegistration(int64_t callId,
                                          const Address &member,
                                          std::auto_ptr<protocol::codec::IAddListenerCodec> addCodec);

                        int64_t getCorrelationId() const;

                        const Address &getMemberAddress() const;

                        const protocol::codec::IAddListenerCodec *getAddCodec() const;

                        void setCorrelationId(int64_t callId);

                    private:
                        int64_t correlationId;
                        Address memberAddress;
                        std::auto_ptr<protocol::codec::IAddListenerCodec> addCodec;
                    };
                }
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif // HAZELCAST_CLIENT_SPI_IMPL_LISTENER_EVENTREGISTRATION_H_
