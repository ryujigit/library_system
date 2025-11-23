#include "../include/Admin.hpp"
#include <string>
#include <mariadb/conncpp.hpp>
#include <memory>
#include <iostream>
#include <vector>

using namespace std;


Admin::Admin(shared_ptr<sql::Connection> dbConn, string id, string usr, string pwd)
    : conn(dbConn), admin_id(move(id)), username(move(usr)), password(move(pwd)) {}

bool Admin::insertDataInDB() {
    try {
        unique_ptr<sql::PreparedStatement> pstmt(
            conn->prepareStatement("INSERT INTO admins (admin_id, username, password) VALUES (?, ?, ?)")
        );
        pstmt->setString(1, admin_id);
        pstmt->setString(2, username);
        pstmt->setString(3, password);
        pstmt->executeUpdate();
        cout << "✅ Admin inserted successfully!\n";
        return true;
    } catch (sql::SQLException &e) {
        cerr << "❌ SQL Error (Insert Admin): " << e.what() << endl;
        return false;
    }
}

bool Admin::insertmember(string member_id, string name, string email, string phone, string password) {
    try {
        unique_ptr<sql::PreparedStatement> pstmt(
            conn->prepareStatement("INSERT INTO members (member_id, name, email, phone, password, issued_books) VALUES (?, ?, ?, ?, ?, ?)")
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
    } catch (sql::SQLException &e) {
        cerr << "❌ SQL Error (Insert Member): " << e.what() << endl;
        return false;
    }
}

bool Admin::deleteMember(const string& member_id) {
    try {
        unique_ptr<sql::PreparedStatement> pstmt(
            conn->prepareStatement("DELETE FROM members WHERE member_id = ?")
        );
        pstmt->setString(1, member_id);
        int affected = pstmt->executeUpdate();

        if (affected > 0) {
            cout << "🗑️  Member " << member_id << " deleted successfully!\n";
            return true;
        } else {
            cout << "⚠️  Member " << member_id << " not found.\n";
            return false;
        }
    } catch (sql::SQLException &e) {
        cerr << "❌ SQL Error (Delete Member): " << e.what() << endl;
        return false;
    }
}

bool Admin::add_booksinDB(const string& book_id, const string& title, const string& author, int total_copies) {
    try {
        unique_ptr<sql::PreparedStatement> pstmt(
            conn->prepareStatement(
                "INSERT INTO books (book_id, title, author, total_copies, available_copies) VALUES (?, ?, ?, ?, ?)"
            )
        );
        pstmt->setString(1, book_id);
        pstmt->setString(2, title);
        pstmt->setString(3, author);
        pstmt->setInt(4, total_copies);
        pstmt->setInt(5, total_copies);
        pstmt->executeUpdate();
        cout << "✅ Book inserted successfully!\n";
        return true;
    } catch (sql::SQLException &e) {
        cerr << "❌ SQL Error (add_booksinDB): " << e.what() << endl;
        return false;
    }
}

bool Admin::issue_book(string loanid, string memberid, string bookid, string ibooks) {
    try {
        unique_ptr<sql::PreparedStatement> check(
            conn->prepareStatement("SELECT available_copies FROM books WHERE book_id = ?")
        );
        check->setString(1, bookid);
        unique_ptr<sql::ResultSet> res(check->executeQuery());
        if (!res->next() || res->getInt("available_copies") <= 0) {
            cout << "⚠️  No available copies for this book.\n";
            return false;
        }

        unique_ptr<sql::PreparedStatement> pstmt(
            conn->prepareStatement("INSERT INTO loans (loan_id, member_id, book_id, issue_date, return_date) VALUES (?, ?, ?, ?, ?)")
        );
        pstmt->setString(1, loanid);
        pstmt->setString(2, memberid);
        pstmt->setString(3, bookid);
        pstmt->setString(4, ibooks);
        pstmt->setNull(5, sql::DataType::VARCHAR);
        pstmt->executeUpdate();

        unique_ptr<sql::PreparedStatement> update(
            conn->prepareStatement("UPDATE books SET available_copies = available_copies - 1 WHERE book_id = ?")
        );
        update->setString(1, bookid);
        update->executeUpdate();

        cout << "✅ Book issued successfully!"<<endl;
        

        unique_ptr<sql::PreparedStatement> update2(
            conn->prepareStatement("UPDATE members set issued_books= issued_books+1 Where member_id = ?")
        );
        update2->setString(1,memberid);
        update2->executeUpdate();

        cout<<" ✅ Member Updated successfully"<<endl;
        return true;
        


    } catch (sql::SQLException &e) {
        cerr << "❌ SQL Error (Issue_Book): " << e.what() << endl;
        return false;
    }
}

bool Admin::return_book(string rdate, string bookid, string memberid) {
    try {
        unique_ptr<sql::PreparedStatement> pstmt(
            conn->prepareStatement("UPDATE loans SET return_date = ? WHERE book_id = ? AND member_id = ? AND return_date IS NULL")
        );
        pstmt->setString(1, rdate);
        pstmt->setString(2, bookid);
        pstmt->setString(3, memberid);
        int affected = pstmt->executeUpdate();

        if (affected > 0) {
            unique_ptr<sql::PreparedStatement> update(
                conn->prepareStatement("UPDATE books SET available_copies = available_copies + 1 WHERE book_id = ?")
            );
            update->setString(1, bookid);
            update->executeUpdate();

            unique_ptr<sql::PreparedStatement> update2(
                conn->prepareStatement("UPDATE members SET issued_books=issued_books-1 WHERE member_id =?")
        );
            update2->setString(1, memberid);
            update2->executeUpdate();
            
            cout << "✅ Book returned successfully!\n";
        } else {
            cout << "⚠️  No active loan found for this member/book.\n";
        }
        return true;
    } catch (sql::SQLException &e) {
        cerr << "❌ SQL Error (Return_Book): " << e.what() << endl;
        return false;
    }
}

void Admin::view_books() const {
    try {
        unique_ptr<sql::PreparedStatement> pstmt(
            conn->prepareStatement("SELECT book_id, title, author, total_copies, available_copies FROM books")
        );
        unique_ptr<sql::ResultSet> res(pstmt->executeQuery());

        cout << "\n📚 All Books (Admin View):\n";
        bool found = false;
        while (res->next()) {
            found = true;
            cout << "Book ID: " << res->getString("book_id") << "\n"
                 << "Title: " << res->getString("title") << "\n"
                 << "Author: " << res->getString("author") << "\n"
                 << "Total Copies: " << res->getInt("total_copies") << "\n"
                 << "Available Copies: " << res->getInt("available_copies") << "\n"
                 << "-----------------------------------\n";
        }
        if (!found)
            cout << "⚠️  No books found in the database.\n";
    } catch (sql::SQLException &e) {
        cerr << "❌ SQL Error (View Books): " << e.what() << endl;
    }
}


std::vector<books> Admin::searchBooksByAuthor(const std::string& author) const {
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

std::vector<member_info> Admin::searchMember(const std::string& memberid) const {
    std::vector<member_info> info;
    try {
        std::unique_ptr<sql::PreparedStatement> pstmt(
            conn->prepareStatement("SELECT member_id, name, email, phone, issued_books FROM members WHERE member_id = ?")
        );
        pstmt->setString(1, memberid);

        std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());

        while (res->next()) {
            member_info M;
            M.member_id = res->getString("member_id");
            M.name = res->getString("name");
            M.email = res->getString("email");
            M.phone = res->getString("phone");
            M.issued_books = res->getInt("issued_books");
            info.push_back(M);
        }
    } catch (sql::SQLException &e) {
        std::cerr << "❌ SQL Error (Search Member): " << e.what() << std::endl;
    }
    return info;
}


std::vector<books> Admin::searchBooksByTitle(const std::string& title) const {
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


std::vector<books> Admin::searchBooksByID(const std::string& ID) const {
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



void Admin::display() const {
    cout << "Admin_ID: " << admin_id << "\n"
         << "Username: " << username << "\n";
}
