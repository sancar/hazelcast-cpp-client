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

#include "ClientTestSupport.h"

#include "hazelcast/client/HazelcastClient.h"
#include "hazelcast/client/ClientConfig.h"
#include "HazelcastServer.h"
#include "HazelcastServerFactory.h"

namespace hazelcast {
    namespace client {
        namespace test {
            class MixedMapTest : public ClientTestSupport {
            protected:
                virtual void TearDown() {
                    // clear maps
                    try {
                        mixedMap->clear();
                    } catch (exception::IException &e) {
                        std::ostringstream out;
                        out << "[TearDown] An exception occured in tear down:" << e.what();
                        util::ILogger::getLogger().warning(out.str());
                    }
                }

                static void SetUpTestCase() {
                    instance = new HazelcastServer(*g_srvFactory);
                    clientConfig = new ClientConfig();
                    clientConfig->addAddress(Address(g_srvFactory->getServerAddress(), 5701));
                    client = new HazelcastClient(*clientConfig);
                    mixedMap = new map::MixedMapProxy(client->getMixedMap("MixedMap"));
                }

                static void TearDownTestCase() {
                    delete mixedMap;
                    delete client;
                    delete clientConfig;
                    delete instance2;
                    delete instance;

                    mixedMap = NULL;
                    client = NULL;
                    clientConfig = NULL;
                    instance2 = NULL;
                    instance = NULL;
                }
                
                static HazelcastServer *instance;
                static HazelcastServer *instance2;
                static ClientConfig *clientConfig;
                static HazelcastClient *client;
                static map::MixedMapProxy *mixedMap;
            };

            HazelcastServer *MixedMapTest::instance = NULL;
            HazelcastServer *MixedMapTest::instance2 = NULL;
            ClientConfig *MixedMapTest::clientConfig = NULL;
            HazelcastClient *MixedMapTest::client = NULL;
            map::MixedMapProxy *MixedMapTest::mixedMap = NULL;

            TEST_F(MixedMapTest, testPutDifferentTypes) {
                mixedMap->put<int, int>(3, 5);
                TypedData oldData = mixedMap->put<int, std::string>(10, "MyStringValue");

                ASSERT_EQ(NULL, oldData.getData());

                TypedData result = mixedMap->get<int>(3);
                ASSERT_EQ(-7, result.getType().typeId);
                std::auto_ptr<int> value = result.get<int>();
                ASSERT_NE((int *)NULL, value.get());
                ASSERT_EQ(5, *value);

                result = mixedMap->get<int>(10);
                ASSERT_EQ(-11, result.getType().typeId);
                std::auto_ptr<std::string> strValue = result.get<std::string>();
                ASSERT_NE((std::string *)NULL, strValue.get());
                ASSERT_EQ("MyStringValue", *strValue);
            }

        }
    }
}
