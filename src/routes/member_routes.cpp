#include "../../include/adminroute.hpp"
#include "../../include/Member.hpp"
#include "crow.h"
#include <iostream>
#include <mariadb/conncpp.hpp>
#include <fstream>
#include <sstream>
#include <memory>
#include <string>
#include <vector>


 void setupMemberRoutes(crow::SimpleApp &app)
 {


    // =======================
    // Member Dashboard Route
    // =======================
    CROW_ROUTE(app, "/member_dashboard")
    .methods("GET"_method)
    ([](const crow::request &req) {
        std::ifstream file("frontend/member_dashboard.html");
        if (!file.is_open())
            return crow::response(404, "member_dashboard.html not found");

        std::ostringstream buffer;
        buffer << file.rdbuf();
        crow::response res(buffer.str());
        res.set_header("Content-Type", "text/html");
        return res;
    });

    CROW_ROUTE(app, "/member/get_books")
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


    CROW_ROUTE(app, "/member/search_Book")
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


    }
 