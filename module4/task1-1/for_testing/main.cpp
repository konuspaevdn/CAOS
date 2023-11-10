#include "main.h"
#include <iostream>

int a = 5;

simple::Foo::Foo() {
    a = 6;
    std::cout << "Happy day\n";
}

int simple::Foo::HocusPocus() {
    return 3;
}

simple::Foo::~Foo() {
    std::cout << "Virtual destructor\n";
}
