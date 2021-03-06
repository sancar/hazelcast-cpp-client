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
// Created by sancar koyunlu on 5/23/13.


#include "hazelcast/client/spi/ClientContext.h"
#include "hazelcast/client/spi/LifecycleService.h"
#include "hazelcast/client/spi/ClusterService.h"
#include "hazelcast/client/protocol/AddMembershipListenerRequest.h"
#include "hazelcast/client/connection/ConnectionManager.h"
#include "hazelcast/client/connection/Connection.h"
#include "hazelcast/client/connection/ClientResponse.h"
#include "hazelcast/client/serialization/pimpl/SerializationService.h"
#include "hazelcast/client/serialization/pimpl/Packet.h"
#include "hazelcast/client/impl/ClientMemberShipEvent.h"
#include "hazelcast/client/impl/SerializableCollection.h"
#include "hazelcast/client/LifecycleEvent.h"
#include "hazelcast/client/spi/ServerListenerService.h"
#include "hazelcast/client/ClientConfig.h"

namespace hazelcast {
    namespace client {
        namespace connection {
            ClusterListenerThread::ClusterListenerThread(spi::ClientContext& clientContext)
            : startLatch(1)
            , clientContext(clientContext)
            , deletingConnection(false) {

            }

            void ClusterListenerThread::staticRun(util::ThreadArgs& args) {
                ClusterListenerThread *clusterListenerThread = (ClusterListenerThread *)args.arg0;
                clusterListenerThread->run(args.currentThread);
            }

            void ClusterListenerThread::setThread(util::Thread *thread) {
                clusterListenerThread.reset(thread);
            }

            void ClusterListenerThread::run(util::Thread *currentThread) {
                while (clientContext.getLifecycleService().isRunning()) {
                    try {
                        if (conn.get() == NULL) {
                            try {
                                conn = clientContext.getClusterService().connectToOne();
                            } catch (std::exception& e) {
                                util::ILogger::getLogger().severe(std::string("Error while connecting to cluster! =>") + e.what());
                                isStartedSuccessfully = false;
                                startLatch.countDown();
                                return;
                            }
                        }
                        loadInitialMemberList();
                        clientContext.getServerListenerService().triggerFailedListeners();
                        isStartedSuccessfully = true;
                        startLatch.countDown();
                        listenMembershipEvents();
                        currentThread->interruptibleSleep(1);
                    } catch (std::exception& e) {
                        if (clientContext.getLifecycleService().isRunning()) {
                            util::ILogger::getLogger().warning(std::string("Error while listening cluster events! -> ") + e.what());
                        }

                        clientContext.getConnectionManager().onCloseOwnerConnection();
                        if (deletingConnection.compareAndSet(false, true)) {
                            util::IOUtil::closeResource(conn.get());
                            conn.reset();
                            deletingConnection = false;
                            clientContext.getLifecycleService().fireLifecycleEvent(LifecycleEvent::CLIENT_DISCONNECTED);
                        }
                        currentThread->interruptibleSleep(1);
                    }

                }

            }


            void ClusterListenerThread::stop() {
                if (deletingConnection.compareAndSet(false, true)) {
                    util::IOUtil::closeResource(conn.get());
                    conn.reset();
                    deletingConnection = false;
                }
                clusterListenerThread->interrupt();
                clusterListenerThread->join();
            }

            std::vector<Address> ClusterListenerThread::getSocketAddresses() {
                std::vector<Address> addresses;
                if (!members.empty()) {
                    std::vector<Address> clusterAddresses = getClusterAddresses();
                    addresses.insert(addresses.begin(), clusterAddresses.begin(), clusterAddresses.end());
                }
                std::vector<Address> configAddresses = getConfigAddresses();
                addresses.insert(addresses.end(), configAddresses.begin(), configAddresses.end());
                return addresses;
            }

            void ClusterListenerThread::loadInitialMemberList() {
                protocol::AddMembershipListenerRequest requestObject;
                boost::shared_ptr<ClientResponse> response = conn->sendAndReceive(requestObject);
                boost::shared_ptr<impl::SerializableCollection> coll = clientContext.getSerializationService().toObject<impl::SerializableCollection>(response->getData());

                std::map<std::string, Member> prevMembers;
                if (!members.empty()) {
                    for (std::vector<Member>::iterator it = members.begin(); it != members.end(); ++it) {
                        prevMembers[(*it).getUuid()] = *it;
                    }
                    members.clear();
                }

                const std::vector<serialization::pimpl::Data>& collection = coll->getCollection();
                for (std::vector<serialization::pimpl::Data>::const_iterator it = collection.begin(); it != collection.end(); ++it) {
                    boost::shared_ptr<Member> member = clientContext.getSerializationService().toObject<Member>(*it);
                    members.push_back(*member);
                }
                updateMembersRef();
                std::vector<MembershipEvent> events;

                for (std::vector<Member>::iterator it = members.begin(); it != members.end(); ++it) {
                    if (prevMembers.count((*it).getUuid()) > 0) {
                        prevMembers.erase((*it).getUuid());
                    } else {
                        events.push_back(MembershipEvent(clientContext.getCluster(), MembershipEvent::MEMBER_ADDED, (*it)));
                    }
                }

                for (std::map<std::string, Member>::iterator it = prevMembers.begin(); it != prevMembers.end(); ++it) {
                    Member& member = it->second;
                    events.push_back(MembershipEvent(clientContext.getCluster(), MembershipEvent::MEMBER_REMOVED, member));
                    if (clientContext.getClusterService().isMemberExists(member.getAddress())) {
                        clientContext.getConnectionManager().removeEndpoint(member.getAddress());
                    }
                }


                for (std::vector<MembershipEvent>::iterator it = events.begin(); it != events.end(); ++it) {
                    clientContext.getClusterService().fireMembershipEvent((*it));
                }
            }

            void ClusterListenerThread::listenMembershipEvents() {
                while (clientContext.getLifecycleService().isRunning()) {
                    serialization::pimpl::Packet packet = conn->readBlocking();
                    serialization::pimpl::Data const& data = packet.getData();
                    if (!clientContext.getLifecycleService().isRunning())
                        break;
                    boost::shared_ptr<connection::ClientResponse> response = clientContext.getSerializationService().toObject<connection::ClientResponse>(data);
                    boost::shared_ptr<impl::ClientMembershipEvent> event = clientContext.getSerializationService().toObject<impl::ClientMembershipEvent>(response->getData());
                    Member member = event->getMember();
                    bool membersUpdated = false;
                    if (event->getEventType() == MembershipEvent::MEMBER_ADDED) {
                        members.push_back(member);
                        membersUpdated = true;
                        clientContext.getConnectionManager().removeEndpoint(member.getAddress());
                    } else if (event->getEventType() == MembershipEvent::MEMBER_REMOVED) {
                        std::vector<Member>::iterator it = std::find(members.begin(), members.end(), member);
                        if (members.end() != it) {
                            members.erase(it);
                        }
                        membersUpdated = true;
                    } else if (event->getEventType() == MembershipEvent::MEMBER_ATTRIBUTE_CHANGED) {
                        impl::MemberAttributeChange const& memberAttributeChange = event->getMemberAttributeChange();
                        for (std::vector<Member>::iterator it = members.begin(); it != members.end(); ++it) {
                            Member& target = *it;
                            if (target.getUuid() == memberAttributeChange.getUuid()) {
                                fireMemberAttributeEvent(memberAttributeChange, target);
                            }
                        }
                    }
                    if (membersUpdated) {
                        updateMembersRef();
                        MembershipEvent membershipEvent(clientContext.getCluster(), event->getEventType(), member);
                        clientContext.getClusterService().fireMembershipEvent(membershipEvent);
                    }
                }
            }


            void ClusterListenerThread::fireMemberAttributeEvent(hazelcast::client::impl::MemberAttributeChange const& memberAttributeChange, Member& target) {
                MemberAttributeEvent::MemberAttributeOperationType operationType = memberAttributeChange.getOperationType();
                const std::string& value = memberAttributeChange.getValue();
                util::IOUtil::PRIMITIVE_ID primitive_id = memberAttributeChange.getTypeId();
                const std::string& key = memberAttributeChange.getKey();
                if (operationType == MemberAttributeEvent::PUT) {//PUT
                    if (primitive_id == util::IOUtil::PRIMITIVE_TYPE_BOOLEAN) {
                        target.setAttribute(key, util::IOUtil::to_value<bool>(value));
                    } else if (primitive_id == util::IOUtil::PRIMITIVE_TYPE_BYTE) {
                        target.setAttribute(key, util::IOUtil::to_value<byte>(value));
                    } else if (primitive_id == util::IOUtil::PRIMITIVE_TYPE_DOUBLE) {
                        target.setAttribute(key, util::IOUtil::to_value<double>(value));
                    } else if (primitive_id == util::IOUtil::PRIMITIVE_TYPE_FLOAT) {
                        target.setAttribute(key, util::IOUtil::to_value<float>(value));
                    } else if (primitive_id == util::IOUtil::PRIMITIVE_TYPE_INTEGER) {
                        target.setAttribute(key, util::IOUtil::to_value<int>(value));
                    } else if (primitive_id == util::IOUtil::PRIMITIVE_TYPE_LONG) {
                        target.setAttribute(key, util::IOUtil::to_value<long>(value));
                    } else if (primitive_id == util::IOUtil::PRIMITIVE_TYPE_SHORT) {
                        target.setAttribute(key, util::IOUtil::to_value<short>(value));
                    } else if (primitive_id == util::IOUtil::PRIMITIVE_TYPE_UTF) {
                        target.setAttribute(key, value);
                    }
                } else if (operationType == MemberAttributeEvent::REMOVE) {//REMOVE
                    if (primitive_id == util::IOUtil::PRIMITIVE_TYPE_BOOLEAN) {
                        target.removeAttribute<bool>(key);
                    } else if (primitive_id == util::IOUtil::PRIMITIVE_TYPE_BYTE) {
                        target.removeAttribute<byte>(key);
                    } else if (primitive_id == util::IOUtil::PRIMITIVE_TYPE_DOUBLE) {
                        target.removeAttribute<double>(key);
                    } else if (primitive_id == util::IOUtil::PRIMITIVE_TYPE_FLOAT) {
                        target.removeAttribute<float>(key);
                    } else if (primitive_id == util::IOUtil::PRIMITIVE_TYPE_INTEGER) {
                        target.removeAttribute<int>(key);
                    } else if (primitive_id == util::IOUtil::PRIMITIVE_TYPE_LONG) {
                        target.removeAttribute<long>(key);
                    } else if (primitive_id == util::IOUtil::PRIMITIVE_TYPE_SHORT) {
                        target.removeAttribute<short>(key);
                    } else if (primitive_id == util::IOUtil::PRIMITIVE_TYPE_UTF) {
                        target.removeAttribute<std::string>(key);
                    }
                }
                MemberAttributeEvent memberAttributeEvent(clientContext.getCluster(), target, operationType, key, value, primitive_id);
                clientContext.getClusterService().fireMemberAttributeEvent(memberAttributeEvent);
            }

            void ClusterListenerThread::updateMembersRef() {
                std::map<Address, Member, addressComparator> map;
                std::stringstream memberInfo;
                memberInfo << std::endl << "Members [" << members.size() << "]  {" << std::endl;
                for (std::vector<Member>::iterator it = members.begin(); it != members.end(); ++it) {
                    (memberInfo << "\t" << (*it) << std::endl);
                    map[(*it).getAddress()] = (*it);
                }
                memberInfo << "}" << std::endl;
                util::ILogger::getLogger().info(memberInfo.str());
                clientContext.getClusterService().setMembers(map);

            }

            std::vector<Address> ClusterListenerThread::getClusterAddresses() const {
                std::vector<Address> socketAddresses;
                for (std::vector<Member>::const_iterator it = members.begin(); it != members.end(); ++it) {
                    socketAddresses.push_back((*it).getAddress());
                }
                return socketAddresses;
            }

            std::vector<Address>  ClusterListenerThread::getConfigAddresses() const {
                std::vector<Address> socketAddresses;
                std::set<Address, addressComparator>& configAddresses = clientContext.getClientConfig().getAddresses();
                std::set<Address, addressComparator>::iterator it;

                for (it = configAddresses.begin(); it != configAddresses.end(); ++it) {
                    socketAddresses.push_back((*it));
                }

                if (socketAddresses.size() == 0) {
                    socketAddresses.push_back(Address("127.0.0.1", 5701));
                }
                std::random_shuffle(socketAddresses.begin(), socketAddresses.end());
                return socketAddresses;
            }
        }
    }
}

