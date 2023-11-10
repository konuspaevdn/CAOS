#include <iostream>

#include "interfaces.h"

#include "for_testing/main.h"

static ClassLoader * Loader = nullptr;

int testFoo()
{
    Class<Foo>* c = reinterpret_cast<Class<Foo>*> (
            Loader->loadClass("Foo"));
    if (c) {
        Foo* instance = c->newInstance();
        std::cout << instance->HocusPocus() << std::endl;
        (void)instance;
        // над уничтожением объекта в этой задаче думать не нужно
        return 0;
    }
    else {
        return 1;
    }
}

std::string GetConstructorId_(const std::string& class_name) {
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

int main()
{
    std::cout << GetConstructorId_("twist::stdlike::mutex") << std::endl;
    /*Loader = new ClassLoader();
    int status = testFoo();
    delete Loader;
    std::cout << status << std::endl;
    return status;*/
}
