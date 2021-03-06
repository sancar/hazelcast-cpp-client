/*
 * Copyright (c) 2008-2015, Hazelcast, Inc. All Rights Reserved.
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
// Created by sancar koyunlu on 6/25/13.




#ifndef HAZELCAST_GetRemainingLeaseRequest
#define HAZELCAST_GetRemainingLeaseRequest


#include "hazelcast/client/impl/ClientRequest.h"

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }
        namespace lock {
            class GetRemainingLeaseRequest : public impl::ClientRequest {
            public:
                GetRemainingLeaseRequest(serialization::pimpl::Data& key);

                int getClassId() const;

                int getFactoryId() const;

                void write(serialization::PortableWriter& writer) const;

                bool isRetryable() const;

            private:
                serialization::pimpl::Data& key;
            };
        }
    }
}


#endif //HAZELCAST_GetRemainingLeaseRequest

