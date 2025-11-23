#ifndef PERSON_HPP
#define PERSON_HPP

#include <iostream>

class Person {
public:
    virtual void view_books() const = 0;
    virtual bool insertDataInDB() = 0;
    virtual void display() const = 0;
    virtual ~Person() = default;
};

#endif
