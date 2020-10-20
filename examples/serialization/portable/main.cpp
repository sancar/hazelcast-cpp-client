/*
 * Copyright (c) 2008-2020, Hazelcast, Inc. All Rights Reserved.
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
#include <hazelcast/Hazelcast.h>
#include <hazelcast/client/HazelcastClient.h>
#include <hazelcast/client/serialization/serialization.h>

struct Person {
    friend std::ostream &operator<<(std::ostream &os, const Person &person);

    std::string name;
    bool male;
    int32_t age;
};

std::ostream &operator<<(std::ostream &os, const Person &person) {
    os << "name: " << person.name << " male: " << person.male << " age: " << person.age;
    return os;
}

namespace hazelcast {
    namespace client {
        namespace serialization {
            template<>
            struct hz_serializer<Person> : portable_serializer {
                static int32_t getFactoryId() noexcept {
                    return 1;
                }

                static int32_t getClassId() noexcept {
                    return 3;
                }

                static void writePortable(const Person &object, hazelcast::client::serialization::PortableWriter &out) {
                    out.write("name", object.name);
                    out.write("gender", object.male);
                    out.write("age", object.age);
                }

                static Person readPortable(hazelcast::client::serialization::PortableReader &in) {
                    return Person{in.read<std::string>("name"), in.read<bool>("gender"), in.read<int32_t>("age")};
                }
            };
        }
    }
}

int main(int argc, char **argv) {
    hazelcast::Hazelcast hz("/Users/sancar/.m2/repository/com/hazelcast/hazelcast/4.1-SNAPSHOT/hazelcast-4.1-SNAPSHOT.jar");

    std::shared_ptr<hazelcast::HazelcastInstance> member1 = hz.newHazelcastInstance();
    std::shared_ptr<hazelcast::HazelcastInstance> member2 = hz.newHazelcastInstance();

    std::cout << std::endl;
    std::cout << member1->getName() << std::endl;
    std::cout << member2->getName() << std::endl;
    std::cout << std::endl;

    {
        auto memberMap = member1->getMap("test");
        memberMap->put<std::string, Person>("foo", Person{"Ihsan", true, 20});

        hazelcast::client::HazelcastClient client;
        auto clientMap = client.getMap("test");

        auto person = clientMap->get<std::string, Person>("foo").get();
        std::cout << std::endl << "Get from client map " << person.get() << std::endl << std::endl;

        client.shutdown();
    }
    {
        auto javaBasedClient = hz.newHazelcastClient();
        auto javaBasedClientMap = javaBasedClient->getMap("test");
        javaBasedClientMap->put<std::string, Person>("foo", Person{"Sancar", true, 30});

        auto person = javaBasedClientMap->get<std::string, Person>("foo");
        std::cout << std::endl << "Get from java based client map " << person.get() << std::endl << std::endl;

        javaBasedClient->shutdown();
    }

    member1->shutdown();
    member2->shutdown();
}



