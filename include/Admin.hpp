#ifndef ADMIN_HPP
#define ADMIN_HPP

#include "Person.hpp"
#include <mariadb/conncpp.hpp>
#include <memory>
#include <string>
using namespace std;


struct books {
    string book_id;
    string title;
    string author;
    int total_copies;
    int available_copies;
};

struct member_info{
    string member_id;
    string name;
    string email;
    string phone;
    string passowrd;
    int issued_books;
};


class Admin : public Person {
private:
    std::string admin_id, username, password;
    std::shared_ptr<sql::Connection> conn;

public:
    Admin(std::shared_ptr<sql::Connection> dbConn, std::string id, std::string usr, std::string pwd);

    bool insertDataInDB() override;
    bool insertmember(std::string member_id, std::string name, std::string email, std::string phone, std::string password);
    bool deleteMember(const std::string& member_id);
    bool add_booksinDB(const std::string& book_id, const std::string& title, const std::string& author, int total_copies);
    bool issue_book(std::string loanid, std::string memberid, std::string bookid, std::string ibooks);
    bool return_book(std::string rdate, std::string bookid, std::string memberid);
    std::vector<books> searchBooksByAuthor(const std::string& author) const;
    std::vector<books> searchBooksByID(const std::string& ID) const;
    std::vector<books> searchBooksByTitle(const std::string& Title) const;
    std::vector<member_info> searchMember(const std::string& memberid) const;

    void view_books() const override;
    void display() const override;
};

#endif
