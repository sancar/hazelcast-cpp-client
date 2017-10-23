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
                    map::MixedMapProxy myMixedMap = client->getMixedMap("MyMixedMap");
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

                class Base : public serialization::IdentifiedDataSerializable {
                public:
                    virtual ~Base() {}

                    virtual int getFactoryId() const {
                        return 666;
                    }

                    virtual int getClassId() const {
                        return 10;
                    }

                    virtual void writeData(serialization::ObjectDataOutput &writer) const {
                    }

                    virtual void readData(serialization::ObjectDataInput &reader) {
                    }
                };
                
                class Derived1 : public Base {
                public:
                    virtual int getClassId() const {
                        return 11;
                    }
                };

                class Derived2 : public Derived1 {
                public:
                    virtual int getClassId() const {
                        return 12;
                    }
                };

                class BasePortable : public serialization::Portable {
                public:
                    virtual ~BasePortable() {}

                    virtual int getFactoryId() const {
                        return 666;
                    }

                    virtual int getClassId() const {
                        return 3;
                    }

                    virtual void writePortable(serialization::PortableWriter &writer) const {
                    }

                    virtual void readPortable(serialization::PortableReader &reader) {
                    }
                };

                class Derived1Portable : public BasePortable {
                public:
                    virtual int getClassId() const {
                        return 4;
                    }
                };

                class Derived2Portable : public BasePortable {
                public:
                    virtual int getClassId() const {
                        return 5;
                    }
                };

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

            TEST_F(MixedMapTest, testPolyMorphismWithIdentifiedDataSerializable) {
                Base base;
                Derived1 derived1;
                Derived2 derived2;
                mixedMap->put<int, Base>(1, base);
                mixedMap->put<int, Derived1>(2, derived1);
                mixedMap->put<int, Derived2>(3, derived2);

                std::set<int> keys;
                keys.insert(1);
                keys.insert(2);
                keys.insert(3);

                std::vector<std::pair<TypedData, TypedData> > values = mixedMap->getAll<int>(keys);
                for (std::vector<std::pair<TypedData, TypedData> >::iterator it = values.begin();it != values.end(); ++it) {
                    TypedData &keyData = (*it).first;
                    TypedData &valueData = (*it).second;
                    std::auto_ptr<int> key = keyData.get<int>();
                    ASSERT_NE((int *)NULL, key.get());
                    serialization::pimpl::ObjectType objectType = valueData.getType();
                    switch (*key) {
                        case 1: {
                            // serialization::pimpl::SerializationConstants::CONSTANT_TYPE_DATA, using -2 since static
                            // variable is not exported into the library
                            ASSERT_EQ(-2, objectType.typeId);
                            ASSERT_EQ(666, objectType.factoryId);
                            ASSERT_EQ(10, objectType.classId);
                            std::auto_ptr<Base> value = valueData.get<Base>();
                            ASSERT_NE((Base *)NULL, value.get());
                            break;
                        }
                        case 2: {
                            ASSERT_EQ(-2, objectType.typeId);
                            ASSERT_EQ(666, objectType.factoryId);
                            ASSERT_EQ(11, objectType.classId);
                            std::auto_ptr<Base> value(valueData.get<Derived1>());
                            ASSERT_NE((Base *)NULL, value.get());
                            break;
                        }
                        case 3: {
                            ASSERT_EQ(-2, objectType.typeId);
                            ASSERT_EQ(666, objectType.factoryId);
                            ASSERT_EQ(12, objectType.classId);
                            std::auto_ptr<Base> value(valueData.get<Derived2>());
                            ASSERT_NE((Base *)NULL, value.get());
                            break;
                        }
                        default:
                            FAIL();
                    }
                }
            }

            TEST_F(MixedMapTest, testPolyMorphismWithPortable) {
                BasePortable base;
                Derived1Portable derived1;
                Derived2Portable derived2;
                mixedMap->put<int, BasePortable>(1, base);
                mixedMap->put<int, Derived1Portable>(2, derived1);
                mixedMap->put<int, Derived2Portable>(3, derived2);

                TypedData secondData = mixedMap->get<int>(2);
                serialization::pimpl::ObjectType secondType = secondData.getType();
                ASSERT_EQ(-1, secondType.typeId);
                secondData.get<Derived1Portable>();
                
                std::set<int> keys;
                keys.insert(1);
                keys.insert(2);
                keys.insert(3);

                std::vector<std::pair<TypedData, TypedData> > values = mixedMap->getAll<int>(keys);
                for (std::vector<std::pair<TypedData, TypedData> >::iterator it = values.begin();it != values.end(); ++it) {
                    TypedData &keyData = (*it).first;
                    TypedData &valueData = (*it).second;
                    std::auto_ptr<int> key = keyData.get<int>();
                    ASSERT_NE((int *)NULL, key.get());
                    serialization::pimpl::ObjectType objectType = valueData.getType();
                    switch (*key) {
                        case 1: {
                            // serialization::pimpl::SerializationConstants::CONSTANT_TYPE_PORTABLE, using -1 since static
                            // variable is not exported into the library
                            ASSERT_EQ(-1, objectType.typeId);
                            ASSERT_EQ(666, objectType.factoryId);
                            ASSERT_EQ(3, objectType.classId);
                            std::auto_ptr<BasePortable> value = valueData.get<BasePortable>();
                            ASSERT_NE((BasePortable *)NULL, value.get());
                            break;
                        }
                        case 2: {
                            ASSERT_EQ(-1, objectType.typeId);
                            ASSERT_EQ(666, objectType.factoryId);
                            ASSERT_EQ(4, objectType.classId);
                            std::auto_ptr<BasePortable> value(valueData.get<Derived1Portable>());
                            ASSERT_NE((BasePortable *)NULL, value.get());
                            break;
                        }
                        case 3: {
                            ASSERT_EQ(-1, objectType.typeId);
                            ASSERT_EQ(666, objectType.factoryId);
                            ASSERT_EQ(5, objectType.classId);
                            std::auto_ptr<BasePortable> value(valueData.get<Derived2Portable>());
                            ASSERT_NE((BasePortable *)NULL, value.get());
                            break;
                        }
                        default:
                            FAIL();
                    }
                }
            }
        }
    }
}
