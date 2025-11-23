#ifndef MEMBER_HPP
#define MEMBER_HPP

#include "Person.hpp"
#include <mariadb/conncpp.hpp>
#include <memory>
#include <string>
#include <vector>
#include <Admin.hpp>

using namespace std;


class Member : public Person {
private:
    std::string member_id, name, email, phone, password;
    std::shared_ptr<sql::Connection> conn;

public:
    Member(std::shared_ptr<sql::Connection> dbConn, std::string id, std::string n, std::string e, std::string p, std::string pass);

    bool insertDataInDB() override;
    std::vector<books> searchBooksByAuthor(const std::string& author) const;
    std::vector<books> searchBooksByID(const std::string& ID) const;
    std::vector<books> searchBooksByTitle(const std::string& Title) const;
    std::vector<books> viewAllBooks() const;

    void view_books() const override;
    void display() const override;
};

#endif
