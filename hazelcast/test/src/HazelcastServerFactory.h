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

#ifndef HAZELCAST_CLIENT_TEST_HAZELCASTSERVERFACTORY_H_
#define HAZELCAST_CLIENT_TEST_HAZELCASTSERVERFACTORY_H_

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#include <crtdefs.h>
#endif

#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>

#include "cpp-controller/RemoteController.h"

#include <memory>

#include <ostream>

#include <hazelcast/util/ILogger.h>
#include <hazelcast/client/Address.h>

using namespace std;
using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;
using namespace hazelcast::client::test::remote;

namespace hazelcast {
    namespace client {
        namespace test {

            class HazelcastServerFactory {
            public:
                HazelcastServerFactory(const std::string &serverXmlConfigFilePath);

                HazelcastServerFactory(const std::string &serverAddress, const std::string &serverXmlConfigFilePath);

                const std::string& getServerAddress();

                Member startServer();

                bool setAttributes(int memberStartOrder);

                bool shutdownServer(const Member &member);

                bool terminateServer(const Member &member);

                ~HazelcastServerFactory();

                RemoteControllerClient &getRemoteController();

                const string &getClusterId() const;

            private:
                util::ILogger logger;
                std::string serverAddress;
                RemoteControllerClient remoteController;
                std::string clusterId;

                std::string readFromXmlFile(const std::string &xmlFilePath);
            };
        }
    }
}

#endif //HAZELCAST_CLIENT_TEST_HAZELCASTSERVERFACTORY_H_

