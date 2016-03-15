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



#include "HazelcastServerFactory.h"
#include "SimpleMapTest.h"
#include "hazelcast/client/HazelcastAll.h"

namespace hazelcast {
    namespace client {
        namespace test {
            HazelcastServerFactory *g_srvFactory = NULL;
        }
    }
}

using namespace std;
using namespace hazelcast::client;
using namespace hazelcast;
std::string str = "this is a test string to test hazel cast";
std::string json_str = "{\"mac\": \"74:e5:43:4b:38:8e\", dddddddddddddaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa ihsan";

/*
\"OF_PORT\": 16, \"IPv4\": \"10.22.41.70\", \"OF_DPID\": 112172508632, \"Records\": [{\"Name\": \"_workstation._tcp.local\", \"Type\": 12, \"Class\": 1, \"ttl\": 4500, \"Domain Name\": \"sachin-HP-ProBook-4430s-2\\032\\09174\\058e5\\05843\\0584b\\05838\\0588e\\093._workstation._tcp.local\"}, {\"Name\": \"_http._tcp.local\", \"Type\": 12, \"Class\": 1, \"ttl\": 4500, \"Domain Name\": \"sachin\\039s\\032remote\\032desktop\\032on\\032sachin-HP-ProBook-4430s\\032\\0352._http._tcp.local\"}, {\"Name\": \"_rfb._tcp.local\", \"Type\": 12, \"Class\": 1, \"ttl\": 4500, \"Domain Name\": \"sachin\\039s\\032remote\\032desktop\\032on\\032sachin-HP-ProBook-4430s\\032\\0352._rfb._tcp.local\"}, {\"Name\": \"sachin-HP-ProBook-4430s-2.local\", \"Type\": 13, \"Class\": 1, \"ttl\": 120, \"OS\": \"LINUX\", \"CPU\": \"X86_64\"}, {\"Data\": 1177097738, \"Name\": \"sachin-HP-ProBook-4430s-2.local\", \"Type\": 1, \"Class\": 1, \"ttl\": 120}, {\"Name\": \"e.8.8.3.b.4.e.f.f.f.3.4.5.e.6.7.0.0.0.0.0.0.0.0.0.0.0.0.0.8.e.f.ip6.arpa\", \"Type\": 12, \"Class\": 1, \"ttl\": 120, \"Domain Name\": \"sachin-HP-ProBook-4430s-2.local\"}, {\"Data\": \"fe80::76e5:43ff:fe4b:388e\", \"Name\": \"sachin-HP-ProBook-4430s-2.local\", \"Type\": 28, \"Class\": 1, \"ttl\": 120}, {\"Name\": \"sachin-HP-ProBook-4430s-2\\032\\09174\\058e5\\05843\\0584b\\05838\\0588e\\093._workstation._tcp.local\", \"Type\": 16, \"Class\": 1, \"txt_str\": [], \"ttl\": 4500}, {\"Name\": \"sachin-HP-ProBook-4430s-2\\032\\09174\\058e5\\05843\\0584b\\05838\\0588e\\093._workstation._tcp.local\", \"Target\": \"sachin-HP-ProBook-4430s-2.local\", \"Type\": 33, \"Class\": 1, \"ttl\": 120, \"Priority\": 0, \"Weight\": 0, \"Port\": 9}, {\"Name\": \"sachin\\039s\\032remote\\032desktop\\032on\\032sachin-HP-ProBook-4430s\\032\\0352._http._tcp.local\", \"Type\": 16, \"Class\": 1, \"txt_str\": [], \"ttl\": 4500}, {\"Name\": \"sachin\\039s\\032remote\\032desktop\\032on\\032sachin-HP-ProBook-4430s\\032\\0352._http._tcp.local\", \"Target\": \"sachin-HP-ProBook-4430s-2.local\", \"Type\": 33, \"Class\": 1, \"ttl\": 120, \"Priority\": 0, \"Weight\": 0, \"Port\": 5800}, {\"Name\": \"sachin\\039s\\032remote\\032desktop\\032on\\032sachin-HP-ProBook-4430s\\032\\0352._rfb._tcp.local\", \"Type\": 16, \"Class\": 1, \"txt_str\": [], \"ttl\": 4500}, {\"Name\": \"sachin\\039s\\032remote\\032desktop\\032on\\032sachin-HP-ProBook-4430s\\032\\0352._rfb._tcp.local\", \"Target\": \"sachin-HP-ProBook-4430s-2.local\", \"Type\": 33, \"Class\": 1, \"ttl\": 120, \"Priority\": 0, \"Weight\": 0, \"Port\": 5900}, {\"Name\": \"70.41.22.10.in-addr.arpa\", \"Type\": 12, \"Class\": 1, \"ttl\": 120, \"Domain Name\": \"sachin-HP-ProBook-4430s-2.local\"}]} ";
*/
int main(int argc, char **argv) {
    ClientConfig clientConfig;
    clientConfig.addAddress(Address("192.168.2.60", 5701));
    clientConfig.setLogLevel(FINEST);
    HazelcastClient client(clientConfig);
    std::cout << "JSON LENGTH " << json_str.length() << std::endl;
    int count = 0;
    IMap<std::string, std::string> map = client.getMap<std::string, std::string>("Hashname");
    while (1) {
        std::cout << "ITERATION " << count++ << std::endl;
        boost::shared_ptr<std::string> value = map.put("Key", str);
        if (NULL != value.get()) {
            util::ILogger::getLogger().info(std::string("First get value:") + *value);
        } else {
            util::ILogger::getLogger().info("First get value is NULL ");
        }
        value = map.put("Key", json_str);
        if (NULL != value.get()) {
            util::ILogger::getLogger().info(std::string("Second get value:") + *value);
        } else {
            util::ILogger::getLogger().info("Second get value is NULL ");
        }
    } // end of while
    return 0;
}

