//
// Created by sancar koyunlu on 9/6/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_CallableTask
#define HAZELCAST_CallableTask

#include "IdentifiedDataSerializable.h"
#include <string>

using namespace hazelcast::client;

class CallableTask : public IdentifiedDataSerializable {
public:
    CallableTask();

    CallableTask(std::string& name);

    int getFactoryId() const;

    int getClassId() const;

    void writeData(serialization::ObjectDataOutput& writer) const;

    void readData(serialization::ObjectDataInput& reader);

private:
    std::string name;

};


#endif //HAZELCAST_CallableTask
