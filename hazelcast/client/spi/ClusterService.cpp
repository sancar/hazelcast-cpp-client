//
// Created by sancar koyunlu on 5/21/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.


#include "ClusterService.h"
#include "HazelcastClient.h"

namespace hazelcast {
    namespace client {
        namespace spi {
            ClusterService::ClusterService(hazelcast::client::HazelcastClient & client)
            : client(client)
            , credentials(client.getClientConfig().getCredentials()) {

            }


        }
    }
}