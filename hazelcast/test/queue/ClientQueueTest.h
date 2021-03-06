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
// Created by sancar koyunlu on 9/4/13.




#ifndef HAZELCAST_ClientQueueTest
#define HAZELCAST_ClientQueueTest

#include "ClientTestSupport.h"
#include "hazelcast/client/ClientConfig.h"
#include "hazelcast/client/IQueue.h"
#include "HazelcastServer.h"

namespace hazelcast {
    namespace client {
        class HazelcastClient;

        namespace test {

            class HazelcastServerFactory;

            class ClientQueueTest : public ClientTestSupport<ClientQueueTest> {
            public:
                ClientQueueTest(HazelcastServerFactory&);

                ~ClientQueueTest();

                void addTests();

                void beforeClass();

                void afterClass();

                void beforeTest();

                void afterTest();

                void testListener();

                void testOfferPoll();

                void testRemainingCapacity();

                void testRemove();

                void testContains();

                void testDrain();

                void testToArray();

                void testAddAll();

                void testRemoveRetain();

                void testClear();

            private:
                HazelcastServer instance;
                ClientConfig clientConfig;
                std::auto_ptr<HazelcastClient> client;
                std::auto_ptr<IQueue<std::string> > q;
            };
        }
    }
}


#endif //HAZELCAST_ClientQueueTest

