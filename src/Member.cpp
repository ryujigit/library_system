#include "../include/Member.hpp"
#include <iomanip>
#include <string>
#include <mariadb/conncpp.hpp>
#include <memory>
#include <iostream>
#include <vector>

using namespace std;

// Constructor




std::vector<books> Member::searchBooksByAuthor(const std::string& author) const {
    std::vector<books> info;
    try {
        std::unique_ptr<sql::PreparedStatement> pstmt(
            conn->prepareStatement("SELECT * FROM books WHERE author LIKE ? ORDER BY author")
        );
        pstmt->setString(1, "%" + author + "%");

        std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
        while (res->next()) {
            books b;
            b.book_id = res->getString("book_id");
            b.title = res->getString("title");
            b.author = res->getString("author");
            b.total_copies = res->getInt("total_copies");
            b.available_copies = res->getInt("available_copies");
            info.push_back(b);
        }
    } catch (sql::SQLException &e) {
        std::cerr << "❌ SQL Error (Search Book By Author): " << e.what() << std::endl;
    }
    return info;
}


Member::Member(shared_ptr<sql::Connection> dbConn, string id, string n, string e, string p, string pass)
    : conn(move(dbConn)), member_id(move(id)), name(move(n)), email(move(e)), phone(move(p)), password(move(pass)) {}

// Display member details

void Member::display() const {
    cout << "Member ID: " << member_id << "\n"
         << "Name: " << name << "\n"
         << "Email: " << email << "\n"
         << "Phone: " << phone << "\n";
}




std::vector<books> Member::searchBooksByTitle(const std::string& title) const {
    std::vector<books> info;
    try {
        std::unique_ptr<sql::PreparedStatement> pstmt(
            conn->prepareStatement("SELECT * FROM books WHERE title LIKE ? ORDER BY title")
        );
        pstmt->setString(1, "%" + title + "%");

        std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());

        while (res->next()) {
            books b;
            b.book_id = res->getString("book_id");
            b.title = res->getString("title");
            b.author = res->getString("author");
            // Assuming `price` exists in the DB but not in your struct — 
            // skip or add it if you want it included later.
            b.total_copies = res->getInt("total_copies");
            b.available_copies = res->getInt("available_copies");
            info.push_back(b);
        }
    } catch (sql::SQLException& e) {
        std::cerr << "❌ SQL Error (Search Book By Title): " << e.what() << std::endl;
    }
    return info;
}



std::vector<books> Member::viewAllBooks() const {
    std::vector<books> info;
    try {
        std::unique_ptr<sql::PreparedStatement> pstmt(
            conn->prepareStatement("SELECT * FROM books ORDER BY title")
        );
        std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());

        while (res->next()) {
            books b;
            b.book_id = res->getString("book_id");
            b.title = res->getString("title");
            b.author = res->getString("author");
            // Add this if your struct includes price:
            // b.price = res->getDouble("price");
            b.total_copies = res->getInt("total_copies");
            b.available_copies = res->getInt("available_copies");
            info.push_back(b);
        }
    } catch (sql::SQLException& e) {
        std::cerr << "❌ SQL Error (View All Books): " << e.what() << std::endl;
    }
    return info;
}




// Insert Member data into DB
bool Member::insertDataInDB() {
    try {
        unique_ptr<sql::PreparedStatement> pstmt(
            conn->prepareStatement(
                "INSERT INTO members (member_id, name, email, phone, password, issued_books) VALUES (?, ?, ?, ?, ?, ?)"
            )
        );
        pstmt->setString(1, member_id);
        pstmt->setString(2, name);
        pstmt->setString(3, email);
        pstmt->setString(4, phone);
        pstmt->setString(5, password);
        pstmt->setInt(6, 0);
        pstmt->executeUpdate();
        cout << "✅ Member inserted successfully!\n";
        return true;
    } catch (sql::SQLException& e) {
        cerr << "❌ SQL Error (Insert Member): " << e.what() << endl;
        return false;
    }
}

// Implement view_books() from Person
void Member::view_books() const {
    cout << "📚 Viewing all available books for Member...\n";
    viewAllBooks();
}


std::vector<books> Member::searchBooksByID(const std::string& ID) const {
    std::vector<books> info;
    try {
        std::unique_ptr<sql::PreparedStatement> pstmt(
            conn->prepareStatement("SELECT * FROM books WHERE book_id = ? ORDER BY title")
        );
        pstmt->setString(1,ID);

        std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());

        while (res->next()) {
            books b;
            b.book_id = res->getString("book_id");
            b.title = res->getString("title");
            b.author = res->getString("author");
            b.total_copies = res->getInt("total_copies");
            b.available_copies = res->getInt("available_copies");
            info.push_back(b);
        }
    } catch (sql::SQLException& e) {
        std::cerr << "❌ SQL Error (Search Book By Title): " << e.what() << std::endl;
    }
    return info;
}