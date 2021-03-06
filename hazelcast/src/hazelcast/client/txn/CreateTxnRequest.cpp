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
// Created by sancar koyunlu on 19/11/13.



#include "hazelcast/client/txn/CreateTxnRequest.h"
#include "hazelcast/client/txn/TxnPortableHook.h"
#include "hazelcast/client/serialization/ObjectDataOutput.h"
#include "hazelcast/client/serialization/PortableWriter.h"

namespace hazelcast {
    namespace client {
        namespace txn {
            CreateTxnRequest::CreateTxnRequest(TransactionOptions options):options(options) {

            }

            int CreateTxnRequest::getFactoryId() const {
                return TxnPortableHook::F_ID;
            }

            int CreateTxnRequest::getClassId() const {
                return TxnPortableHook::CREATE;
            }

            void CreateTxnRequest::write(serialization::PortableWriter &writer) const {
                BaseTxnRequest::write(writer);
                serialization::ObjectDataOutput &out = writer.getRawDataOutput();
                options.writeData(out);
                out.writeBoolean(false);//SerializableXID null
            }

        }
    }
}

