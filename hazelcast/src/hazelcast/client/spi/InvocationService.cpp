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
//
// Created by sancar koyunlu on 5/23/13.

#include "hazelcast/client/protocol/codec/ClientPingCodec.h"
#include "hazelcast/util/Util.h"
#include "hazelcast/client/protocol/ResponseMessageConst.h"
#include "hazelcast/client/protocol/codec/ErrorCodec.h"
#include "hazelcast/client/spi/InvocationService.h"
#include "hazelcast/client/spi/ClusterService.h"
#include "hazelcast/client/spi/PartitionService.h"
#include "hazelcast/client/connection/ConnectionManager.h"
#include "hazelcast/client/connection/CallPromise.h"
#include "hazelcast/client/ClientConfig.h"
#include "hazelcast/client/spi/ClientContext.h"
#include "hazelcast/client/connection/CallFuture.h"
#include "hazelcast/client/ClientProperties.h"
#include "hazelcast/client/connection/Connection.h"
#include "hazelcast/client/spi/ServerListenerService.h"
#include "hazelcast/client/serialization/pimpl/SerializationService.h"
#include "hazelcast/client/exception/IllegalStateException.h"
#include "hazelcast/client/exception/InstanceNotActiveException.h"

#include <assert.h>
#include <string>

namespace hazelcast {
    namespace client {
        namespace spi {
            InvocationService::InvocationService(spi::ClientContext &clientContext)
                    : clientContext(clientContext), isOpen(false) {
                redoOperation = clientContext.getClientConfig().isRedoOperation();
                ClientProperties &properties = clientContext.getClientProperties();
                retryWaitTime = properties.getRetryWaitTime().getInteger();
                retryCount = properties.getRetryCount().getInteger();
                heartbeatTimeout = properties.getHeartbeatTimeout().getInteger();
                if (retryWaitTime <= 0) {
                    retryWaitTime = util::IOUtil::to_value<int>(ClientProperties::PROP_REQUEST_RETRY_WAIT_TIME_DEFAULT);
                }
                if (retryCount <= 0) {
                    retryCount = util::IOUtil::to_value<int>(ClientProperties::PROP_REQUEST_RETRY_COUNT_DEFAULT);
                }
                if (heartbeatTimeout <= 0) {
                    heartbeatTimeout = util::IOUtil::to_value<int>(
                            ClientProperties::PROP_REQUEST_RETRY_WAIT_TIME_DEFAULT);
                }
            }

            InvocationService::~InvocationService() {

            }

            bool InvocationService::start() {
                return isOpen.compareAndSet(false, true);
            }

            void InvocationService::shutdown() {
                isOpen.compareAndSet(true, false);
            }

            connection::CallFuture  InvocationService::invokeOnRandomTarget(
                    std::auto_ptr<protocol::ClientMessage> request) {
                return invokeOnRandomTarget(request, NULL);
            }

            connection::CallFuture  InvocationService::invokeOnPartitionOwner(
                    std::auto_ptr<protocol::ClientMessage> request, int partitionId) {
                return invokeOnPartitionOwner(request, NULL, partitionId);
            }

            connection::CallFuture  InvocationService::invokeOnTarget(std::auto_ptr<protocol::ClientMessage> request,
                                                                      const Address &address) {
                return invokeOnTarget(request, NULL, address);
            }

            connection::CallFuture InvocationService::invokeOnRandomTarget(
                    std::auto_ptr<protocol::ClientMessage> request,
                    client::impl::BaseEventHandler *eventHandler) {
                std::auto_ptr<client::impl::BaseEventHandler> managedEventHandler(eventHandler);
                hazelcast::util::SharedPtr<connection::Connection> connection = clientContext.getConnectionManager().getRandomConnection(
                        retryCount);
                return doSend(request, managedEventHandler, connection, -1);
            }

            connection::CallFuture  InvocationService::invokeOnTarget(std::auto_ptr<protocol::ClientMessage> request,
                                                                      client::impl::BaseEventHandler *eventHandler,
                                                                      const Address &address) {
                std::auto_ptr<client::impl::BaseEventHandler> managedEventHandler(eventHandler);
                hazelcast::util::SharedPtr<connection::Connection> connection = clientContext.getConnectionManager().getOrConnect(
                        address, retryCount);
                return doSend(request, managedEventHandler, connection, -1);
            }

            connection::CallFuture  InvocationService::invokeOnPartitionOwner(
                    std::auto_ptr<protocol::ClientMessage> request, client::impl::BaseEventHandler *handler,
                    int partitionId) {
                hazelcast::util::SharedPtr<Address> owner = clientContext.getPartitionService().getPartitionOwner(partitionId);
                if (owner.get() != NULL) {
                    hazelcast::util::SharedPtr<connection::Connection> connection = clientContext.getConnectionManager().getOrConnect(
                            *owner, retryCount);
                    std::auto_ptr<client::impl::BaseEventHandler> managedEventHandler(handler);
                    return doSend(request, managedEventHandler, connection, partitionId);
                }
                return invokeOnRandomTarget(request, handler);
            }

            connection::CallFuture  InvocationService::invokeOnConnection(
                    std::auto_ptr<protocol::ClientMessage> request,
                    hazelcast::util::SharedPtr<connection::Connection> connection) {
                return doSend(request, std::auto_ptr<client::impl::BaseEventHandler>(NULL), connection, -1);
            }

            bool InvocationService::isRedoOperation() const {
                return redoOperation;
            }

            int InvocationService::getRetryWaitTime() const {
                return retryWaitTime;
            }

            int InvocationService::getRetryCount() const {
                return retryCount;
            }

            void InvocationService::removeEventHandler(int64_t callId) {
                std::vector<hazelcast::util::SharedPtr<connection::Connection> > connections = clientContext.getConnectionManager().getConnections();
                std::vector<hazelcast::util::SharedPtr<connection::Connection> >::iterator it;
                for (it = connections.begin(); it != connections.end(); ++it) {
                    hazelcast::util::SharedPtr<connection::Connection> &connectionPtr = *it;
                    if (deRegisterEventHandler(*connectionPtr, callId).get() != NULL) {
                        return;
                    }
                }
            }


            connection::CallFuture  InvocationService::doSend(std::auto_ptr<protocol::ClientMessage> request,
                                                              std::auto_ptr<client::impl::BaseEventHandler> eventHandler,
                                                              hazelcast::util::SharedPtr<connection::Connection> connection,
                                                              int partitionId) {
                request->setPartitionId(partitionId);
                hazelcast::util::SharedPtr<connection::CallPromise> promise(new connection::CallPromise());
                promise->setRequest(request);
                promise->setEventHandler(eventHandler);

                hazelcast::util::SharedPtr<connection::Connection> conn = registerAndEnqueue(connection, promise);
                return connection::CallFuture(promise, conn, heartbeatTimeout, this);
            }

            bool InvocationService::isAllowedToSentRequest(connection::Connection &connection,
                                                           protocol::ClientMessage const &request) {
                util::ILogger &logger = util::ILogger::getLogger();
                if (!connection.live) {
                    if (logger.isEnabled(FINEST)) {
                        std::stringstream message;
                        message << " Connection(" << connection.getRemoteEndpoint();
                        message << ") is closed, won't write packet with callId : " << request.getCorrelationId();
                        logger.finest(message.str());
                    }
                    return false;
                }

                if (!connection.isHeartBeating()) {
                    if (protocol::codec::ClientPingCodec::RequestParameters::TYPE == request.getMessageType()) {
                        return true;
                    }
                    std::stringstream message;
                    if (logger.isEnabled(FINEST)) {
                        message << " Connection(" << connection.getRemoteEndpoint();
                        message << ") is not heart-beating, won't write packet with callId : " <<
                        request.getCorrelationId();
                        logger.finest(message.str());
                    }
                    return false;
                }
                return true;
            }

            hazelcast::util::SharedPtr<connection::Connection> InvocationService::resend(
                    hazelcast::util::SharedPtr<connection::CallPromise> promise, const std::string &lastTriedAddress) {
                // reset the future, shall avoid future set twice warning message
                promise->resetFuture();

                if (promise->getRequest()->isBindToSingleConnection()) {
                    std::auto_ptr<exception::IException> exception(new exception::HazelcastInstanceNotActiveException(lastTriedAddress));
                    promise->setException(exception);
                    return hazelcast::util::SharedPtr<connection::Connection>();
                }
                if (promise->incrementAndGetResendCount() > getRetryCount()) {
                    std::auto_ptr<exception::IException> exception(new exception::HazelcastInstanceNotActiveException(lastTriedAddress));
                    promise->setException(exception);
                    return hazelcast::util::SharedPtr<connection::Connection>();
                }

                hazelcast::util::SharedPtr<connection::Connection> connection;
                try {
                    connection::ConnectionManager &cm = clientContext.getConnectionManager();
                    connection = cm.getRandomConnection(getRetryCount(), lastTriedAddress, getRetryWaitTime());
                } catch (exception::IException &) {
                    std::auto_ptr<exception::IException> exception(new exception::HazelcastInstanceNotActiveException(lastTriedAddress));
                    promise->setException(exception);
                    return hazelcast::util::SharedPtr<connection::Connection>();
                }

                int64_t correlationId = promise->getRequest()->getCorrelationId();

                hazelcast::util::SharedPtr<connection::Connection> actualConn = registerAndEnqueue(connection, promise);

                if (NULL != actualConn.get()) {
                    char msg[300];
                    const Address &serverAddr = connection->getRemoteEndpoint();
                    hazelcast::util::snprintf(msg, 300, "[InvocationService::resend] Re-sending the request with id %lld "
                                                      "originally destined for %s to server [%s:%d] using the new correlation id %lld", correlationId,
                                              lastTriedAddress.c_str(), serverAddr.getHost().c_str(), serverAddr.getPort(), promise->getRequest()->getCorrelationId());
                    util::ILogger::getLogger().info(msg);
                }

                return actualConn;
            }

            hazelcast::util::SharedPtr<connection::Connection> InvocationService::registerAndEnqueue(
                    hazelcast::util::SharedPtr<connection::Connection> &connection,
                    hazelcast::util::SharedPtr<connection::CallPromise> promise) {
                if (!isOpen) {
                    char msg[200];
                    util::snprintf(msg, 200, "[InvocationService::registerAndEnqueue] InvocationService is shutdown. "
                            "Did not register the promise for message correlation id:%lld",
                                   promise->getRequest()->getCorrelationId());
                    hazelcast::util::ILogger::getLogger().info(msg);

                    std::auto_ptr<exception::IException> exception(new exception::IllegalStateException(
                            "InvocationService::registerAndEnqueue", "Invocation service is not open. Can not process the request."));
                    promise->setException(exception);

                    return hazelcast::util::SharedPtr<connection::Connection>();
                }

                registerCall(*connection, promise); //Don't change the order with following line

                protocol::ClientMessage *request = promise->getRequest();

                if (!isAllowedToSentRequest(*connection, *request)) {
                    deRegisterCall(connection->getConnectionId(), request->getCorrelationId());
                    std::string address = util::IOUtil::to_string(connection->getRemoteEndpoint());

                    // slow down the resend to avoid infinite loop until the connection is closed
                    // If connection is not closed, then the code may come to this point during resend and it shall
                    // easily cause the stack to grow
                    util::sleepmillis(100);

                    return resend(promise, address);
                }

                connection->write(request);
                return connection;
            }

            void InvocationService::registerCall(connection::Connection &connection,
                                                 hazelcast::util::SharedPtr<connection::CallPromise> promise) {
                int64_t callId = clientContext.getConnectionManager().getNextCallId();
                promise->getRequest()->setCorrelationId(callId);
                if (getCallPromiseMap(connection.getConnectionId())->put(callId, promise).get()) {
                    std::ostringstream out;
                    out << "[InvocationService::registerCall] The call id map already contains the promise for call "
                            "id:" << callId << ". This is unexpected!!!";
                    hazelcast::util::ILogger::getLogger().severe(out.str());
                    assert(0); // just fail in debug mode
                }
                if (promise->getEventHandler() != NULL) {
                    registerEventHandler(callId, connection, promise);
                }
            }

            hazelcast::util::SharedPtr<connection::CallPromise> InvocationService::deRegisterCall(
                    int connectionId, int64_t callId) {
                return getCallPromiseMap(connectionId)->remove(callId);
            }

            void InvocationService::registerEventHandler(int64_t correlationId, connection::Connection &connection,
                                                         hazelcast::util::SharedPtr<connection::CallPromise> promise) {
                getEventHandlerPromiseMap(connection)->put(correlationId, promise);
            }

            void InvocationService::handleMessage(connection::Connection &connection,
                                                  std::auto_ptr<protocol::ClientMessage> message) {
                int64_t correlationId = message->getCorrelationId();
                if (message->isFlagSet(protocol::ClientMessage::LISTENER_EVENT_FLAG)) {
                    hazelcast::util::SharedPtr<connection::CallPromise> promise = getEventHandlerPromise(connection,
                                                                                                correlationId);
                    if (promise.get() != NULL) {
                        promise->getEventHandler()->handle(message);
                    }
                    return;
                }

                int connId = connection.getConnectionId();
                const Address &serverAddr = connection.getRemoteEndpoint();
                hazelcast::util::SharedPtr<connection::CallPromise> promise = deRegisterCall(connId, correlationId);
                if (NULL == promise.get()) {
                    if (connection.live) {
                        std::ostringstream out;
                        out << "[InvocationService::handleMessage] Could not find the promise for correlation id:" <<
                                correlationId << ". It may have been re-sent.";
                        hazelcast::util::ILogger::getLogger().finest(out.str());
                    }

                    /** Do not proceed if no promise exist for the message, just drop it since it is most probably re-sent
                    * on another connection.
                    **/
                    return;
                }

                if (protocol::codec::ErrorCodec::TYPE == message->getMessageType()) {
                    std::auto_ptr<exception::IException> exception = exceptionFactory.createException(*message);

                    std::string addrString = util::IOUtil::to_string(serverAddr);
                    tryResend(exception, promise, addrString);
                    return;
                }

                if (!handleEventUuid(message.get(), promise))
                    return; //if response is event uuid,then return.

                promise->setResponse(message);
            }

            /* returns shouldSetResponse */
            bool InvocationService::handleEventUuid(protocol::ClientMessage *response,
                                                    hazelcast::util::SharedPtr<connection::CallPromise> promise) {
                client::impl::BaseEventHandler *eventHandler = promise->getEventHandler();
                if (eventHandler != NULL) {
                    if (eventHandler->registrationId.size() ==
                        0) //if uuid is not set, it means it is first time that we are getting uuid.
                        return true;                    // then no need to handle it, just set as normal response

                    // result->registrationId is the alias for the original registration
                    clientContext.getServerListenerService().reRegisterListener(eventHandler->registrationId, response);

                    return false;
                }
                //if it does not have event handler associated with it, then it is a normal response.
                return true;
            }

            void InvocationService::tryResend(std::auto_ptr<exception::IException> exception,
                                              hazelcast::util::SharedPtr<connection::CallPromise> promise,
                                              const std::string &lastTriedAddress) {
                bool serviceOpen = isOpen;
                if (serviceOpen && (promise->getRequest()->isRetryable() || isRedoOperation())) {
                    resend(promise, lastTriedAddress);
                    return;
                }
                // At this point the exception may have been already set at the promise, hence we need to reset it
                // and set the exception
                promise->resetException(exception);
            }

            hazelcast::util::SharedPtr<connection::CallPromise> InvocationService::getEventHandlerPromise(
                    connection::Connection &connection, int64_t callId) {
                return getEventHandlerPromiseMap(connection)->get(callId);
            }

            hazelcast::util::SharedPtr<connection::CallPromise> InvocationService::deRegisterEventHandler(
                    connection::Connection &connection, int64_t callId) {
                return getEventHandlerPromiseMap(connection)->remove(callId);
            }

            void InvocationService::cleanResources(connection::Connection &connection) {
                std::vector<std::pair<int64_t, hazelcast::util::SharedPtr<connection::CallPromise> > > promises = getCallPromiseMap(
                        connection.getConnectionId())->clear();

                std::string address = util::IOUtil::to_string(connection.getRemoteEndpoint());

                char msg[200];
                util::snprintf(msg, 200, "[cleanResources] There are %u waiting promises on connection with id:%d (%s) ",
                               promises.size(), connection.getConnectionId(), address.c_str());
                util::ILogger::getLogger().info(msg);

                for (std::vector<std::pair<int64_t, hazelcast::util::SharedPtr<connection::CallPromise> > >::iterator it = promises.begin();
                     it != promises.end(); ++it) {
                    if (!isOpen) {
                        std::auto_ptr<exception::IException> exception(new exception::IllegalStateException(
                                "InvocationService::cleanResources", "Invocation service is not open."));
                        it->second->setException(exception);
                    } else {
                        std::auto_ptr<exception::IException> exception(new exception::IOException(
                                "InvocationService::cleanResources", "Connection closed."));
                        tryResend(exception, it->second, address);
                    }
                }

                cleanEventHandlers(connection);
            }

            void InvocationService::cleanEventHandlers(connection::Connection &connection) {
                std::vector<std::pair<int64_t, hazelcast::util::SharedPtr<connection::CallPromise> > > promises = getEventHandlerPromiseMap(
                        connection)->clear();

                util::ILogger &logger = util::ILogger::getLogger();

                char msg[200];
                util::snprintf(msg, 200, "[InvocationService::cleanEventHandlers] There are %ld event handler promises on connection with id:%d to be retried",
                               promises.size(), connection.getConnectionId());
                logger.info(msg);

                if (isOpen) {
                    for (std::vector<std::pair<int64_t, hazelcast::util::SharedPtr<connection::CallPromise> > >::const_iterator it = promises.begin();
                         it != promises.end(); ++it) {
                        clientContext.getServerListenerService().retryFailedListener(it->second);
                    }
                } else {
                    logger.info("[InvocationService::cleanEventHandlers] The service is closed. Shall not retry "
                                        "registering any event handler if exists.");
                }
            }

            hazelcast::util::SharedPtr<util::SynchronizedMap<int64_t, connection::CallPromise> > InvocationService::getCallPromiseMap(
                    int connectionId) {
                return callPromises.getOrPutIfAbsent(connectionId);
            }

            hazelcast::util::SharedPtr<util::SynchronizedMap<int64_t, connection::CallPromise> > InvocationService::getEventHandlerPromiseMap(
                    connection::Connection &connection) {
                return eventHandlerPromises.getOrPutIfAbsent(connection.getConnectionId());
            }
        }
    }
}
