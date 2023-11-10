#include <iostream>
#include "interfaces.h"

extern "C" {
#include <dlfcn.h>
#include <unistd.h>
}

typedef void (*constructor_t)(void*);

struct ClassImpl {
public:
    constructor_t constr = nullptr;
};

AbstractClass::AbstractClass() : pImpl(new ClassImpl) {
}

AbstractClass::~AbstractClass() {
    delete pImpl;
}

void* AbstractClass::newInstanceWithSize(size_t sizeofClass) {
    char* obj = new char[sizeofClass];
    pImpl->constr(obj);
    return obj;
}

struct ClassLoaderImpl {
public:
    ClassLoaderError err = ClassLoaderError::NoError;
    void* lib = nullptr;
};

ClassLoader::ClassLoader() : pImpl(new ClassLoaderImpl) {
}

ClassLoader::~ClassLoader() {
    if (pImpl->lib) {
        dlclose(pImpl->lib);
    }
    delete pImpl;
}

std::string GetConstructorId(const std::string& class_name) {
    std::string id = "_ZN";
    size_t len = 0;
    std::string buffer;
    for (size_t i = 0; i < class_name.size(); ++i) {
        if (i != class_name.size() - 1 && class_name[i] == ':' && class_name[i + 1] == ':') {
            id += std::to_string(len) + buffer;
            len = 0;
            buffer.clear();
            ++i;
            continue;
        }
        ++len;
        buffer.push_back(class_name[i]);
    }
    id += std::to_string(len) + buffer + std::string("C1Ev");
    return id;
}

AbstractClass* ClassLoader::loadClass(const std::string& fullyQualifiedName) {
    std::string relative_path = fullyQualifiedName;
    for (size_t i = 1; i < relative_path.size(); ++i) {
        if (relative_path[i - 1] == ':' && relative_path[i] == ':') {
            relative_path[i - 1] = '/';
            relative_path[i] = '/';
        }
    }
    std::string full_path = std::string(std::getenv("CLASSPATH")) + std::string("/") + relative_path
            + std::string(".so");
    if (-1 == access(full_path.c_str(), F_OK)) {
        std::cerr << "Can't find given file\n";
        pImpl->err = ClassLoaderError::FileNotFound;
        return nullptr;
    }
    void* lib = dlopen(full_path.c_str(), RTLD_NOW | RTLD_LOCAL);
    if (!lib) {
        std::cerr << "Can't load library\n";
        pImpl->err = ClassLoaderError::LibraryLoadError;
        return nullptr;
    }
    pImpl->lib = lib;
    auto* ptr = new AbstractClass;
    std::string constr_id = GetConstructorId(fullyQualifiedName);
    void* constr = dlsym(lib, constr_id.c_str());
    if (!constr) {
        std::cerr << "Can't locate class constructor\n";
        pImpl->err = ClassLoaderError::NoClassInLibrary;
        return nullptr;
    }
    ptr->pImpl->constr = (constructor_t)constr;
    return ptr;
}

ClassLoaderError ClassLoader::lastError() const {
    return pImpl->err;
}