//
// Created by sancar koyunlu on 6/21/13.
// Copyright (c) 2013 hazelcast. All rights reserved.

#ifndef HAZELCAST_ITEM_EVENT
#define HAZELCAST_ITEM_EVENT

#include "hazelcast/client/impl/EventObject.h"
#include "hazelcast/client/connection/Member.h"

namespace hazelcast {
    namespace client {
        class HAZELCAST_API ItemEventType {
        public:
            enum Type {
                ADDED = 1, REMOVED = 2
            } value;

            ItemEventType() {

            }

            ItemEventType(Type value)
            :value(value) {

            }

            operator int() const {
                return value;
            }

            void operator = (int i) {
                switch (i) {
                    case 1:
                        value = ADDED;
                        break;
                    case 2:
                        value = REMOVED;
                        break;
                }
            }

        };

        template <typename E>
        class HAZELCAST_API ItemEvent : public impl::EventObject {
        public:

            ItemEvent(const std::string &name, ItemEventType eventType, const E &item, const connection::Member &member)
            : impl::EventObject(name)
            , member(member)
            , eventType(eventType)
            , item(item) {

            };


            const E &geItem() const {
                return item;
            };

            /**
             * Returns the member fired this event.
             *
             * @return the member fired this event.
             */
            connection::Member getMember() const {
                return member;
            };

            /**
             * Return the event type
             *
             * @return event type
             */
            ItemEventType getEventType() const {
                return eventType;
            };

            /**
             * Returns the name of the map for this event.
             *
             * @return name of the map.
             */
            std::string getName() const {
                return getSource();
            };

        private:
            E item;
            ItemEventType eventType;
            connection::Member member;

        };
    }
}

#endif //HAZELCAST_ITEM_EVENT