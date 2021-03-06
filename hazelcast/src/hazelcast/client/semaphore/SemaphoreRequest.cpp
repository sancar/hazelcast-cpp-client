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
// Created by sancar koyunlu on 20/11/13.



#include "hazelcast/client/semaphore/SemaphoreRequest.h"
#include "hazelcast/client/serialization/PortableWriter.h"
#include "hazelcast/client/semaphore/SemaphorePortableHook.h"

namespace hazelcast {
    namespace client {
        namespace semaphore {
            SemaphoreRequest::SemaphoreRequest(const std::string& instanceName, int permitCount)
                :instanceName(instanceName)
                , permitCount(permitCount) {

                }

                int SemaphoreRequest::getFactoryId() const {
                    return SemaphorePortableHook::F_ID;
                }


                void SemaphoreRequest::write(serialization::PortableWriter& writer) const {
                    writer.writeUTF("n", &instanceName);
                    writer.writeInt("p", permitCount);
                }

        }
    }
}
