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
#include <hazelcast/client/Hazelcast.h>
#include <hazelcast/client/HazelcastClient.h>
#include <hazelcast/client/serialization/serialization.h>

struct Person {
    friend std::ostream &operator<<(std::ostream &os, const Person &person);

    std::string name;
    bool male;
    int32_t age;
};

void test_multiple_server_threads(std::shared_ptr<hazelcast::IMap> map, int num_threads, int num_ops, size_t object_size);

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
    if (argc != 4) {
        std::cout << "USAGE: portable <num_threads> <total_num_ops> <object_size(bytes)>\n";
        return -1;
    }

    auto num_threads = std::atoi(argv[1]);
    auto num_ops = std::atoi(argv[2]);

    size_t object_size = static_cast<size_t>(std::atoi(argv[3]));

    std::cout << "Using parameters: "
                 "\n\tnum_threads:" << num_threads <<
                 "\n\ttotal_num_ops:" << num_ops <<
                 "\n\tobject_size:" << object_size << "\n";

    hazelcast::Hazelcast hz(
            "/Users/ihsan/.m2/repository/com/hazelcast/hazelcast/4.1-SNAPSHOT/hazelcast-4.1-SNAPSHOT.jar");
    std::shared_ptr<hazelcast::HazelcastInstance> member1 = hz.newHazelcastInstance();
    std::shared_ptr<hazelcast::HazelcastInstance> member2 = hz.newHazelcastInstance();

    std::cout << std::endl;
    std::cout << member1->getName() << std::endl;
    std::cout << member2->getName() << std::endl;
    std::cout << std::endl;


    hazelcast::client::HazelcastClient client;
    std::shared_ptr<hazelcast::client::IMap> clientMap = client.getMap("test");

    std::shared_ptr<hazelcast::IMap> memberMap = member1->getMap("test");

    test_multiple_server_threads(memberMap, num_threads, num_ops, object_size);

    memberMap->put<std::string, Person>("foo", Person{"Ihsan", true, 20});

    boost::optional<Person> person = clientMap->get<std::string, Person>("foo").get();
    if (person.has_value()) {
        std::cout << std::endl << "Get from client map " << person.get() << std::endl << std::endl;
    } else {
        std::cout << std::endl << "Get from client map Empty" << std::endl << std::endl;
    }

    std::shared_ptr<hazelcast::HazelcastInstance> javaBasedClient = hz.newHazelcastClient();
    std::shared_ptr<hazelcast::IMap> javaBasedClientMap = javaBasedClient->getMap("test");
    boost::optional<Person> person2 = javaBasedClientMap->put<std::string, Person>("foo", Person{"Sancar", true, 30});
    javaBasedClientMap->put<std::string, Person>("foo", Person{"Sancar", true, 30});
    if (person2.has_value()) {
        std::cout << std::endl << "Put from java based client map " << person2.get() << std::endl << std::endl;
    } else {
        std::cout << std::endl << "Put from java based client map Empty" << std::endl << std::endl;
    }

    person = javaBasedClientMap->get<std::string, Person>("foo");
    if (person.has_value()) {
        std::cout << std::endl << "Get from java based client map " << person.get() << std::endl << std::endl;
    } else {
        std::cout << std::endl << "Get from java based client map Empty" << std::endl << std::endl;
    }

    client.shutdown();
    javaBasedClient->shutdown();
    member1->shutdown();
    member2->shutdown();

    std::cout << "TEST FINISH" << std::endl;
}

void test_multiple_server_threads(std::shared_ptr<hazelcast::IMap> map, int num_threads, int num_ops, size_t object_size) {
    std::vector<std::thread> threads(num_threads);
    std::vector<hazelcast::byte> value(object_size);
    auto start = std::chrono::steady_clock::now();
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([=] () {
            for (int j = 0; j < num_ops; ++j) {
                map->put(j, value);
            }
        });
    }

    for (auto &t : threads) {
        t.join();
    }
    auto end = std::chrono::steady_clock::now();
    auto duration_msec = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << "Total time for server api:" << duration_msec << " msecs. " << ((double)num_ops * 1000 / duration_msec) << " ops/sec.";
}
