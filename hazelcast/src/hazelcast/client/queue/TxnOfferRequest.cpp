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
// Created by sancar koyunlu on 8/5/13.



#include "hazelcast/client/queue/TxnOfferRequest.h"
#include "hazelcast/client/queue/QueuePortableHook.h"
#include "hazelcast/client/serialization/PortableWriter.h"

namespace hazelcast {
    namespace client {
        namespace queue {
            TxnOfferRequest::TxnOfferRequest(const std::string& name, long timeoutInMillis, const serialization::pimpl::Data& data)
            : name(name)
            , data(data)
            , timeoutInMillis(timeoutInMillis) {

            }

            int TxnOfferRequest::getFactoryId() const {
                return QueuePortableHook::F_ID;
            }

            int TxnOfferRequest::getClassId() const {
                return QueuePortableHook::TXN_OFFER;
            }

            void TxnOfferRequest::write(serialization::PortableWriter& writer) const {
                BaseTxnRequest::write(writer);
                writer.writeUTF("n", &name);
                writer.writeLong("t", timeoutInMillis);
                serialization::ObjectDataOutput& out = writer.getRawDataOutput();
                out.writeData(&data);
            }

        }
    }
}
