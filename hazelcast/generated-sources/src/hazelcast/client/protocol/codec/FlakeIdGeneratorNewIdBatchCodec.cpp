/*
 * Copyright (c) 2008-2019, Hazelcast, Inc. All Rights Reserved.
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

#include "hazelcast/util/Util.h"
#include "hazelcast/util/ILogger.h"

#include "hazelcast/client/protocol/codec/FlakeIdGeneratorNewIdBatchCodec.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const FlakeIdGeneratorMessageType FlakeIdGeneratorNewIdBatchCodec::REQUEST_TYPE = HZ_FLAKEIDGENERATOR_NEWIDBATCH;
                const bool FlakeIdGeneratorNewIdBatchCodec::RETRYABLE = true;
                const ResponseMessageConst FlakeIdGeneratorNewIdBatchCodec::RESPONSE_TYPE = (ResponseMessageConst) 126;

                std::unique_ptr<ClientMessage> FlakeIdGeneratorNewIdBatchCodec::encodeRequest(
                        const std::string &name, 
                        int32_t batchSize) {
                    int32_t requiredDataSize = calculateDataSize(name, batchSize);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t)FlakeIdGeneratorNewIdBatchCodec::REQUEST_TYPE);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(batchSize);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t FlakeIdGeneratorNewIdBatchCodec::calculateDataSize(
                        const std::string &name, 
                        int32_t batchSize) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(batchSize);
                    return dataSize;
                }

                    FlakeIdGeneratorNewIdBatchCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {
                        
                        

                    base = clientMessage.get<int64_t >();
                            

                    increment = clientMessage.get<int64_t >();
                            

                    batchSize = clientMessage.get<int32_t >();
                            
                    }

                    FlakeIdGeneratorNewIdBatchCodec::ResponseParameters FlakeIdGeneratorNewIdBatchCodec::ResponseParameters::decode(ClientMessage &clientMessage) {
                        return FlakeIdGeneratorNewIdBatchCodec::ResponseParameters(clientMessage);
                    }


            }
        }
    }
}

