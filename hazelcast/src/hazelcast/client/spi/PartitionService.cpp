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
// Created by sancar koyunlu on 6/3/13.



#include "hazelcast/client/spi/PartitionService.h"
#include "hazelcast/client/spi/ClusterService.h"
#include "hazelcast/client/spi/LifecycleService.h"
#include "hazelcast/client/spi/InvocationService.h"
#include "hazelcast/client/impl/GetPartitionsRequest.h"
#include "hazelcast/client/impl/PartitionsResponse.h"
#include "hazelcast/client/serialization/pimpl/SerializationService.h"
#include "hazelcast/client/spi/ClientContext.h"
#include "hazelcast/client/exception/IllegalStateException.h"
#include "hazelcast/client/connection/CallFuture.h"
#include <climits>

namespace hazelcast {
    namespace client {
        namespace spi {
            PartitionService::PartitionService(spi::ClientContext& clientContext)
            : clientContext(clientContext)
            , updating(false)
            , partitionCount(0) {

            }

            bool PartitionService::start() {
                util::LockGuard lg(startLock);
                if (!getInitialPartitions()) {
                    return false;
                }
                util::Thread *partitionListener = new util::Thread("hz.partitionListener", PartitionService::staticRunListener, this);
                partitionListenerThread.reset(partitionListener);
                return true;
            }

            void PartitionService::shutdown() {
                util::LockGuard lg(startLock);
                if (partitionListenerThread.get() != NULL) {
                    partitionListenerThread->interrupt();
                    partitionListenerThread->join();
                }
            }

            boost::shared_ptr<Address> PartitionService::getPartitionOwner(int partitionId) {
                return partitions.get(partitionId);
            }

            int PartitionService::getPartitionId(const serialization::pimpl::Data& key) {
                const int pc = partitionCount;
                int hash = key.getPartitionHash();
                return (hash == INT_MIN) ? 0 : abs(hash) % pc;
            }

            void PartitionService::staticRunListener(util::ThreadArgs& args) {
                PartitionService *partitionService = (PartitionService *)args.arg0;
                partitionService->runListener(args.currentThread);
            }

            void PartitionService::runListener(util::Thread *currentThread) {
                while (clientContext.getLifecycleService().isRunning()) {
                    try {
                        currentThread->interruptibleSleep(10);
                        if (!clientContext.getLifecycleService().isRunning()) {
                            break;
                        }
                        runRefresher();
                    } catch (exception::IException& e) {
                        util::ILogger::getLogger().warning(std::string("PartitionService::runListener") + e.what());
                    }
                }
            }

            void PartitionService::runRefresher() {

                if (updating.compareAndSet(false, true)) {
                    try {
                        boost::shared_ptr<impl::PartitionsResponse> partitionResponse;
                        std::auto_ptr<Address> ptr = clientContext.getClusterService().getMasterAddress();
                        if (ptr.get() == NULL) {
                            partitionResponse = getPartitionsFrom();
                        } else {
                            partitionResponse = getPartitionsFrom(*ptr.get());
                        }
                        if (partitionResponse != NULL) {
                            processPartitionResponse(*partitionResponse);
                        }
                    } catch (hazelcast::client::exception::IException& e) {
                        util::ILogger::getLogger().finest(std::string("Exception partitionService::runRefresher ") + e.what());
                    } catch (...) {
                        util::ILogger::getLogger().finest(std::string("Unkown exception partitionService::runRefresher "));
                        throw;
                    }
                    updating = false;
                }

            }

            boost::shared_ptr<impl::PartitionsResponse> PartitionService::getPartitionsFrom(const Address& address) {
                impl::GetPartitionsRequest *request = new impl::GetPartitionsRequest();
                boost::shared_ptr<impl::PartitionsResponse> partitionResponse;
                try {
                    connection::CallFuture future = clientContext.getInvocationService().invokeOnTarget(request, address);
                    partitionResponse = clientContext.getSerializationService().toObject<impl::PartitionsResponse>(future.get());
                } catch (exception::IOException& e) {
                    util::ILogger::getLogger().severe(std::string("Error while fetching cluster partition table => ") + e.what());
                }
                return partitionResponse;
            }

            boost::shared_ptr<impl::PartitionsResponse>PartitionService::getPartitionsFrom() {
                impl::GetPartitionsRequest *request = new impl::GetPartitionsRequest();
                boost::shared_ptr<impl::PartitionsResponse> partitionResponse;
                try {
                    connection::CallFuture future = clientContext.getInvocationService().invokeOnRandomTarget(request);
                    partitionResponse = clientContext.getSerializationService().toObject<impl::PartitionsResponse>(future.get());
                } catch (exception::IOException& e) {
                    util::ILogger::getLogger().warning(std::string("Error while fetching cluster partition table => ") + e.what());
                }
                return partitionResponse;
            }

            void PartitionService::processPartitionResponse(impl::PartitionsResponse& response) {
                const std::vector<Address>& members = response.getMembers();
                const std::vector<int>& ownerIndexes = response.getOwnerIndexes();
                if (partitionCount == 0) {
                    partitionCount = ownerIndexes.size();
                }
                for (int partitionId = 0; partitionId < (int)partitionCount; ++partitionId) {
                    int ownerIndex = ownerIndexes[partitionId];
                    if (ownerIndex > -1) {
                        boost::shared_ptr<Address> address(new Address(members[ownerIndex]));
                        partitions.put(partitionId, address);
                    }
                }
            }

            bool PartitionService::getInitialPartitions() {
                std::vector<Member> memberList = clientContext.getClusterService().getMemberList();
                for (std::vector<Member>::iterator it = memberList.begin(); it < memberList.end(); ++it) {
                    Address target = (*it).getAddress();
                    boost::shared_ptr<impl::PartitionsResponse> response = getPartitionsFrom(target);
                    if (response != NULL) {
                        processPartitionResponse(*response);
                        return true;
                    }
                }
                util::ILogger::getLogger().severe("PartitionService::getInitialPartitions Cannot get initial partitions!");
                return false;
            }


            int PartitionService::getPartitionCount() {
                return partitionCount;
            }
        }
    }
}

