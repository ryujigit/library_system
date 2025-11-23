#include "../../include/adminroute.hpp"
#include "../../include/Admin.hpp"
#include "crow.h"
#include <iostream>
#include <mariadb/conncpp.hpp>
#include <fstream>
#include <sstream>
#include <memory>
#include <string>
#include <vector>

using namespace std;





void setupAdminRoutes(crow::SimpleApp &app)
{
    // =======================
    // Admin Dashboard Route
    // =======================
    CROW_ROUTE(app, "/admin_dashboard")
    .methods("GET"_method)
    ([](const crow::request &req) {
        std::ifstream file("frontend/admin_dashboard.html");
        if (!file.is_open())
            return crow::response(404, "admin_dashboard.html not found");

        std::ostringstream buffer;
        buffer << file.rdbuf();
        crow::response res(buffer.str());
        res.set_header("Content-Type", "text/html");
        return res;
    });

    // =======================
    // Get Books Route
    // =======================
    CROW_ROUTE(app, "/admin/get_books")
    .methods("POST"_method)
    ([](const crow::request &req) {
        try {
            sql::Driver *driver = sql::mariadb::get_driver_instance();
            sql::SQLString url("jdbc:mariadb://localhost:3306/library_system");
            sql::Properties props({{"user", "admin"}, {"password", "admin"}});
            shared_ptr<sql::Connection> conn(driver->connect(url, props));

            string query = "SELECT * FROM books";
            unique_ptr<sql::PreparedStatement> pstmt(conn->prepareStatement(query));
            unique_ptr<sql::ResultSet> res(pstmt->executeQuery());

            std::vector<crow::json::wvalue> books;

            while (res->next()) {
                crow::json::wvalue book;
                book["book_id"] = res->getString("book_id");
                book["title"] = res->getString("title");
                book["author"] = res->getString("author");
                book["total_copies"] = res->getInt("total_copies");
                book["available_copies"] = res->getInt("available_copies");
                books.push_back(std::move(book));
            }

            crow::json::wvalue result_json;
            result_json["status"] = "success";
            result_json["books"] = std::move(books);

            crow::response response(result_json);
            response.set_header("Content-Type", "application/json");
            return response;
        }
        catch (const sql::SQLException &e) {
            crow::json::wvalue err;
            err["status"] = "error";
            err["message"] = string("Database error: ") + e.what();
            crow::response res(500, err.dump());
            res.set_header("Content-Type", "application/json");
            return res;
        }
    });

    // =======================
    // Insert Book Route
    // =======================
    CROW_ROUTE(app, "/admin/insertBook")
    .methods("POST"_method)
    ([](const crow::request &req) {
        try {
            auto x = crow::json::load(req.body);
            if (!x)
                return crow::response(400, "Invalid JSON");

            sql::Driver *driver = sql::mariadb::get_driver_instance();
            sql::SQLString url("jdbc:mariadb://localhost:3306/library_system");
            sql::Properties props({{"user", "admin"}, {"password", "admin"}});
            shared_ptr<sql::Connection> conn(driver->connect(url, props));

            Admin admin(conn, "A001", "admin", "admin");

            bool success = admin.add_booksinDB(
                x["book_id"].s(),
                x["title"].s(),
                x["author"].s(),
                x["total_copies"].i()
            );

            crow::json::wvalue response_json;
            if (success) {
                response_json["status"] = "success";
                response_json["message"] = "Book added successfully";
                crow::response res(200, response_json.dump());
                res.set_header("Content-Type", "application/json");
                return res;
            } else {
                response_json["status"] = "error";
                response_json["message"] = "Failed to add book (check server logs)";
                crow::response res(500, response_json.dump());
                res.set_header("Content-Type", "application/json");
                return res;
            }
        }
        catch (const sql::SQLException &e) {
            crow::json::wvalue err;
            err["status"] = "error";
            err["message"] = string("Database error: ") + e.what();
            crow::response res(500, err.dump());
            res.set_header("Content-Type", "application/json");
            return res;
        }
    });




    CROW_ROUTE(app, "/admin/addmember")
        .methods("POST"_method)
        ([](const crow::request &req) {
        try {
            auto x = crow::json::load(req.body);
            if (!x) {
            return crow::response(400, "Invalid JSON");
            }


        // Validate required fields
        const std::vector<std::string> required_fields = {"member_id", "name", "email", "phone", "password"};
        for (const auto& field : required_fields) {
            if (!x.has(field)) {
                crow::json::wvalue err;
                err["status"] = "error";
                err["message"] = "Missing field: " + field;
                crow::response res(400, err.dump());
                res.set_header("Content-Type", "application/json");
                return res;
            }
        }

        sql::Driver* driver = sql::mariadb::get_driver_instance();
        sql::SQLString url("jdbc:mariadb://localhost:3306/library_system");
        sql::Properties props({{"user", "admin"}, {"password", "admin"}});
        std::shared_ptr<sql::Connection> conn(driver->connect(url, props));

        Admin admin(conn, "A001", "admin", "admin");

        bool success = admin.insertmember(
            x["member_id"].s(),
            x["name"].s(),
            x["email"].s(),
            x["phone"].s(),
            x["password"].s()
        );

        crow::json::wvalue response_json;
        if (success) {
            response_json["status"] = "success";
            response_json["message"] = "Member added successfully";
            crow::response res(200, response_json.dump());
            res.set_header("Content-Type", "application/json");
            return res;
        } else {
            response_json["status"] = "error";
            response_json["message"] = "Failed to add member (check server logs)";
            crow::response res(500, response_json.dump());
            res.set_header("Content-Type", "application/json");
            return res;
        }
    }
    catch (const sql::SQLException &e) {
        crow::json::wvalue err;
        err["status"] = "error";
        err["message"] = std::string("Database error: ") + e.what();
        crow::response res(500, err.dump());
        res.set_header("Content-Type", "application/json");
        return res;
    }
    catch (const std::exception &e) {
        crow::json::wvalue err;
        err["status"] = "error";
        err["message"] = std::string("Unexpected error: ") + e.what();
        crow::response res(500, err.dump());
        res.set_header("Content-Type", "application/json");
        return res;
    }
});

 CROW_ROUTE(app, "/admin/deletemember")
    .methods("POST"_method)
    ([](const crow::request &req) {
        try {
            auto x = crow::json::load(req.body);
            if (!x)
                return crow::response(400, "Invalid JSON");

            sql::Driver *driver = sql::mariadb::get_driver_instance();
            sql::SQLString url("jdbc:mariadb://localhost:3306/library_system");
            sql::Properties props({{"user", "admin"}, {"password", "admin"}});
            shared_ptr<sql::Connection> conn(driver->connect(url, props));

            Admin admin(conn, "A001", "admin", "admin"); 
            bool success = admin.deleteMember(
               x["member_id"].s()
            );

            crow::json::wvalue response_json;
            if (success) {
                response_json["status"] = "success";
                response_json["message"] = "Member deleted successfully";
                crow::response res(200, response_json.dump());
                res.set_header("Content-Type", "application/json");
                return res;
            } else {
                response_json["status"] = "error";
                response_json["message"] = "Failed to Delete member (check server logs)";
                crow::response res(500, response_json.dump());
                res.set_header("Content-Type", "application/json");
                return res;
            }
        }
        catch (const sql::SQLException &e) {
            crow::json::wvalue err;
            err["status"] = "error";
            err["message"] = string("Database error: ") + e.what();
            crow::response res(500, err.dump());
            res.set_header("Content-Type", "application/json");
            return res;
        }
    });





    CROW_ROUTE(app, "/admin/issue_book")
    .methods("POST"_method)
    ([](const crow::request &req) {
        try {
            auto x = crow::json::load(req.body);
            if (!x)
                return crow::response(400, "Invalid JSON");

            sql::Driver *driver = sql::mariadb::get_driver_instance();
            sql::SQLString url("jdbc:mariadb://localhost:3306/library_system");
            sql::Properties props({{"user", "admin"}, {"password", "admin"}});
            shared_ptr<sql::Connection> conn(driver->connect(url, props));

            Admin admin(conn, "A001", "admin", "admin");

            bool success = admin.issue_book(
                x["loan_id"].s(),
                x["member_id"].s(),
                x["book_id"].s(),
                x["issue_date"].s()
            );

            crow::json::wvalue response_json;
            if (success) {
                response_json["status"] = "success";
                response_json["message"] = "Book issued successfully";
                crow::response res(200, response_json.dump());
                res.set_header("Content-Type", "application/json");
                return res;
            } else {
                response_json["status"] = "error";
                response_json["message"] = "Failed to issue book (check server logs)";
                crow::response res(500, response_json.dump());
                res.set_header("Content-Type", "application/json");
                return res;
            }
        }
        catch (const sql::SQLException &e) {
            crow::json::wvalue err;
            err["status"] = "error";
            err["message"] = string("Database error: ") + e.what();
            crow::response res(500, err.dump());
            res.set_header("Content-Type", "application/json");
            return res;
        }
    });



     CROW_ROUTE(app, "/admin/return_book")
    .methods("POST"_method)
    ([](const crow::request &req) {
        try {
            auto x = crow::json::load(req.body);
            if (!x)
                return crow::response(400, "Invalid JSON");

            sql::Driver *driver = sql::mariadb::get_driver_instance();
            sql::SQLString url("jdbc:mariadb://localhost:3306/library_system");
            sql::Properties props({{"user", "admin"}, {"password", "admin"}});
            shared_ptr<sql::Connection> conn(driver->connect(url, props));

            Admin admin(conn, "A001", "admin", "admin");

    

            bool success = admin.return_book(
                x["rdate"].s(),
                x["bookid"].s(),
                x["memberid"].s()
            );

            crow::json::wvalue response_json;
            if (success) {
                response_json["status"] = "success";
                response_json["message"] = "Book returned successfully";
                crow::response res(200, response_json.dump());
                res.set_header("Content-Type", "application/json");
                return res;
            } else {
                response_json["status"] = "error";
                response_json["message"] = "Failed to return book (check server logs)";
                crow::response res(500, response_json.dump());
                res.set_header("Content-Type", "application/json");
                return res;
            }
        }
        catch (const sql::SQLException &e) {
            crow::json::wvalue err;
            err["status"] = "error";
            err["message"] = string("Database error: ") + e.what();
            crow::response res(500, err.dump());
            res.set_header("Content-Type", "application/json");
            return res;
        }
    });



  CROW_ROUTE(app, "/admin/search_Book")
    .methods("POST"_method)
    ([](const crow::request &req) {
        try {
            auto x = crow::json::load(req.body);
            if (!x)
                return crow::response(400, "Invalid JSON");

            sql::Driver *driver = sql::mariadb::get_driver_instance();
            sql::SQLString url("jdbc:mariadb://localhost:3306/library_system");
            sql::Properties props({{"user", "admin"}, {"password", "admin"}});
            std::shared_ptr<sql::Connection> conn(driver->connect(url, props));

            Admin admin(conn, "A001", "admin", "admin");
            std::vector<books> results;

            std::string choice = x["choice"].s();

            if (choice == "author") {
                results = admin.searchBooksByAuthor(x["author"].s());
            } 
            else if (choice == "title") {
                results = admin.searchBooksByTitle(x["title"].s());
            } 
            else if (choice == "id") {
                results = admin.searchBooksByID(x["id"].s());
            } 
            else {
                return crow::response(400, "Invalid search choice");
            }

            crow::json::wvalue result;
            result["status"] = "success";

        crow::json::wvalue::list bookList;
            for (const auto &b : results) {
                crow::json::wvalue book_json;
                book_json["book_id"] = b.book_id;
                book_json["title"] = b.title;
                book_json["author"] = b.author;
                book_json["total_copies"] = b.total_copies;
                book_json["available_copies"] = b.available_copies;
                bookList.push_back(std::move(book_json));
}

result["data"] = std::move(bookList);

crow::response res(200, result.dump());
res.set_header("Content-Type", "application/json");
return res;

        } 
        catch (const sql::SQLException &e) {
            crow::json::wvalue err;
            err["status"] = "error";
            err["message"] = std::string("Database error: ") + e.what();
            crow::response res(500, err.dump());
            res.set_header("Content-Type", "application/json");
            return res;
        } 
        catch (const std::exception &e) {
            crow::json::wvalue err;
            err["status"] = "error";
            err["message"] = std::string("Internal error: ") + e.what();
            crow::response res(500, err.dump());
            res.set_header("Content-Type", "application/json");
            return res;
        }
    });



CROW_ROUTE(app, "/admin/member_info")
.methods("POST"_method)
([](const crow::request &req) {
    try {
        auto x = crow::json::load(req.body);
        if (!x)
            return crow::response(400, "Invalid JSON");

        sql::Driver *driver = sql::mariadb::get_driver_instance();
        sql::SQLString url("jdbc:mariadb://localhost:3306/library_system");
        sql::Properties props({{"user", "admin"}, {"password", "admin"}});
        std::shared_ptr<sql::Connection> conn(driver->connect(url, props));

        Admin admin(conn, "A001", "admin", "admin");

        // Get member info using Admin class
        std::vector<member_info> results = admin.searchMember(x["member_id"].s());

        // Prepare response JSON
        crow::json::wvalue result;
        result["status"] = "success";

        // Create a JSON list (array)
        result["data"] = crow::json::wvalue::list();

        int i = 0;
        for (const auto &m : results) {
        crow::json::wvalue member_json;
        member_json["member_id"] = m.member_id;
        member_json["name"] = m.name;
        member_json["email"] = m.email;
        member_json["phone"] = m.phone;
        member_json["issued_books"] = m.issued_books;

         // Assign to the array by index
         result["data"][i++] = std::move(member_json);
}

crow::response res(200, result.dump());
res.set_header("Content-Type", "application/json");
return res;

    } 
    catch (const sql::SQLException &e) {
        crow::json::wvalue err;
        err["status"] = "error";
        err["message"] = std::string("Database error: ") + e.what();
        crow::response res(500, err.dump());
        res.set_header("Content-Type", "application/json");
        return res;
    } 
    catch (const std::exception &e) {
        crow::json::wvalue err;
        err["status"] = "error";
        err["message"] = std::string("Internal error: ") + e.what();
        crow::response res(500, err.dump());
        res.set_header("Content-Type", "application/json");
        return res;
    }
});



}


