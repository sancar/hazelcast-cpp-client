//
//  TestNamedPortable.h
//  Server
//
//  Created by sancar koyunlu on 1/11/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef Server_TestNamedPortable_h
#define Server_TestNamedPortable_h


#include <iostream>
#include "hazelcast/client/serialization/PortableReader.h"
#include "hazelcast/client/serialization/PortableWriter.h"
#include "hazelcast/client/serialization/Portable.h"

using namespace hazelcast::client::serialization;

class TestNamedPortable : public Portable {
public:
    TestNamedPortable() {
    };

    TestNamedPortable(std::string name, int k):name(name), k(k) {
    };

    virtual ~TestNamedPortable(){

    };

    int getClassId() {
        return 3;
    };

    int getFactoryId() {
        return 1;
    };

    void writePortable(PortableWriter& writer) {
        writer.writeUTF("name", name);
        writer.writeInt("myint", k);
    };

    void readPortable(PortableReader& reader) {
        name = reader.readUTF("name");
        k = reader.readInt("myint");
    };

    bool operator ==(TestNamedPortable& m) {
        if (this == &m)
            return true;
        if (k != m.k)
            return false;
        if (name.compare(m.name))
            return false;
        return true;
    };

    bool operator !=(TestNamedPortable& m) {
        return !(*this == m);
    };
    std::string name;
    int k;
};

#endif
