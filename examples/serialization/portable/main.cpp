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

void test_multiple_threads(hazelcast::Hazelcast &hz, std::function<void(int key, const std::vector<hazelcast::byte> &)> f, int num_threads, int num_ops, size_t object_size) {
    std::vector<std::thread> threads;
    std::vector<hazelcast::byte> value(object_size);

    for (int j = 0; j < num_ops; ++j) {
        f(j, value);
    }

    auto start = std::chrono::steady_clock::now();
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([=, &hz] () {
            for (int j = 0; j < num_ops; ++j) {
                f(j, value);
            }
            hz.detach_current_thread();
        });
    }

    for (auto &t : threads) {
        t.join();
    }
    auto end = std::chrono::steady_clock::now();
    auto duration_msec = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << "Total time: " << duration_msec << " msecs. " << ((double) num_threads * num_ops * 1000 / duration_msec) << " ops/sec.\n\n";
}

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
    if (argc != 5) {
        std::cout << "USAGE: portable <num_threads> <total_num_ops> <object_size(bytes)> <type>\n";
        return -1;
    }

    auto num_threads = std::atoi(argv[1]);
    auto num_ops = std::atoi(argv[2]);

    size_t object_size = static_cast<size_t>(std::atoi(argv[3]));
    char* type = argv[4];

    std::cout << "Using parameters: "
                 "\n\tnum_threads:" << num_threads <<
                 "\n\ttotal_num_ops:" << num_ops <<
                 "\n\tobject_size:" << object_size <<
                 "\n\ttype:" << type << "\n";

    hazelcast::Hazelcast hz(
            "/Users/sancar/.m2/repository/com/hazelcast/hazelcast/4.1-SNAPSHOT/hazelcast-4.1-SNAPSHOT.jar");


    if(strcmp(type, "lite") == 0) {
        std::cout << "Lite member:\n";
        std::shared_ptr<hazelcast::HazelcastInstance> liteMember = hz.newHazelcastInstance("<hazelcast xmlns=\"http://www.hazelcast.com/schema/config\"\n"
                                                                                           "           xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"\n"
                                                                                           "           xsi:schemaLocation=\"http://www.hazelcast.com/schema/config\n"
                                                                                           "           http://www.hazelcast.com/schema/config/hazelcast-config-4.1.xsd\">\n"
                                                                                           "    <lite-member enabled=\"true\"/>\n"
                                                                                           "</hazelcast>");

        std::shared_ptr<hazelcast::IMap> memberMap = liteMember->getMap("test");
        test_multiple_threads(hz, [=](int key, const std::vector<hazelcast::byte> &value) {
            memberMap->put(key, value);
        }, num_threads, num_ops, object_size);

        liteMember->shutdown();

    }

    if(strcmp(type, "java") == 0) {
        std::cout << "Java client:\n";
        std::shared_ptr<hazelcast::HazelcastInstance> javaBasedClient = hz.newHazelcastClient();
        std::shared_ptr<hazelcast::IMap> javaBasedClientMap = javaBasedClient->getMap("test");

        test_multiple_threads(hz, [=](int key, const std::vector<hazelcast::byte> &value) {
            javaBasedClientMap->put(key, value);
        }, num_threads, num_ops, object_size);

        javaBasedClient->shutdown();
    }

    if(strcmp(type, "cpp") == 0) {
        hazelcast::client::HazelcastClient client;
        std::shared_ptr<hazelcast::client::IMap> clientMap = client.getMap("test");

        std::cout << "C++ client:\n";
        test_multiple_threads(hz, [=](int key, const std::vector<hazelcast::byte> &value) {
            clientMap->put(key, value).get();
        }, num_threads, num_ops, object_size);

        client.shutdown();
    }

    std::cout << "TEST FINISH" << std::endl;
}


