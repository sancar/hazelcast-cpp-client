//
//  PortableContext.cpp
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//
#include "hazelcast/client/serialization/pimpl/PortableContext.h"
#include "hazelcast/client/serialization/pimpl/ClassDefinitionContext.h"
#include "hazelcast/client/serialization/pimpl/SerializationService.h"

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                PortableContext::PortableContext(int version)
                : contextVersion(version)
                , serializerHolder(*this) {
                };

                bool PortableContext::isClassDefinitionExists(int factoryId, int classId) {
                    return isClassDefinitionExists(factoryId, classId, contextVersion);
                };

                boost::shared_ptr<ClassDefinition> PortableContext::lookup(int factoryId, int classId) {
                    return getClassDefinitionContext(factoryId).lookup(classId, contextVersion);
                };

                bool PortableContext::isClassDefinitionExists(int factoryId, int classId, int version) {
                    return getClassDefinitionContext(factoryId).isClassDefinitionExists(classId, version);
                };

                boost::shared_ptr<ClassDefinition> PortableContext::lookup(int factoryId, int classId, int version) {
                    return getClassDefinitionContext(factoryId).lookup(classId, version);
                };

                boost::shared_ptr<ClassDefinition> PortableContext::createClassDefinition(int factoryId, std::auto_ptr<std::vector<byte> > binary) {
                    return getClassDefinitionContext(factoryId).createClassDefinition(binary);
                };

                void PortableContext::registerNestedDefinitions(boost::shared_ptr<ClassDefinition> cd) {
                    std::vector<boost::shared_ptr<ClassDefinition> > nestedDefinitions = cd->getNestedClassDefinitions();
                    for (std::vector<boost::shared_ptr<ClassDefinition> >::iterator it = nestedDefinitions.begin(); it < nestedDefinitions.end(); it++) {
                        registerClassDefinition(*it);
                        registerNestedDefinitions(*it);
                    }
                };

                boost::shared_ptr<ClassDefinition> PortableContext::registerClassDefinition(boost::shared_ptr<ClassDefinition> cd) {
                    return getClassDefinitionContext(cd->getFactoryId()).registerClassDefinition(cd);
                };

                int PortableContext::getVersion() {
                    return contextVersion;
                };

                SerializerHolder& PortableContext::getSerializerHolder() {
                    return serializerHolder;
                };

                ClassDefinitionContext& PortableContext::getClassDefinitionContext(int factoryId) {
                    boost::shared_ptr<ClassDefinitionContext> value = classDefContextMap.get(factoryId);
                    if (value == NULL) {
                        value = boost::shared_ptr<ClassDefinitionContext>(new ClassDefinitionContext(this));
                        boost::shared_ptr<ClassDefinitionContext> current = classDefContextMap.putIfAbsent(factoryId, value);
                        if (current != NULL) {
                            value = current;
                        }
                    }
                    return *value;
                };
            }
        }
    }
}
