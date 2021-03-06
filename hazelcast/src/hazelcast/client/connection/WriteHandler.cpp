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
// Created by sancar koyunlu on 25/12/13.
//

#include "hazelcast/client/connection/WriteHandler.h"
#include "hazelcast/client/connection/OutSelector.h"
#include "hazelcast/client/connection/ConnectionManager.h"
#include "hazelcast/client/connection/Connection.h"
#include "hazelcast/client/exception/IOException.h"
#include "hazelcast/client/serialization/pimpl/Packet.h"
//#define BOOST_THREAD_PROVIDES_FUTURE

namespace hazelcast {
    namespace client {
        namespace connection {
            WriteHandler::WriteHandler(Connection &connection, OutSelector &oListener, size_t bufferSize)
            : IOHandler(connection, oListener)
            , buffer(new char[bufferSize])
            , byteBuffer(buffer, bufferSize)
            , lastData(NULL)
            , ready(false)
            , informSelector(true)
            {
            }


            WriteHandler::~WriteHandler() {
                serialization::pimpl::Packet *packet;
                while ((packet = writeQueue.poll()) != NULL) {
                    delete packet;
                }
                delete [] buffer;
            }

            void WriteHandler::run() {
                informSelector = true;
                if (ready) {
                    handle();
                } else {
                    registerHandler();
                }
                ready = false;
            }

            void WriteHandler::enqueueData(serialization::pimpl::Packet *packet) {
                writeQueue.offer(packet);
                if (informSelector.compareAndSet(true, false)) {
                    ioSelector.addTask(this);
                    ioSelector.wakeUp();
                }
            }

            void WriteHandler::handle() {
                if (!connection.live) {
                    return;
                }

                if (lastData == NULL) {
                    lastData = writeQueue.poll();
                    if (lastData == NULL && byteBuffer.position() == 0) {
                        ready = true;
                        return;
                    }
                }
                while (byteBuffer.hasRemaining() && lastData != NULL) {
                    bool complete = lastData->writeTo(byteBuffer);
                    if (complete) {
                        delete lastData;
                        lastData = NULL;
                        lastData = writeQueue.poll();
                    } else {
                        break;
                    }
                }

                if (byteBuffer.position() > 0) {
                    byteBuffer.flip();
                    try {
                        byteBuffer.writeTo(connection.getSocket());
                    } catch (exception::IOException &e) {
                        delete lastData;
                        lastData = NULL;
                        handleSocketException(e.what());
                        return;
                    }
                    if (byteBuffer.hasRemaining()) {
                        byteBuffer.compact();
                    } else {
                        byteBuffer.clear();
                    }
                }
                ready = false;
                registerHandler();

            }
        }
    }
}

