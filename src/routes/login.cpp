#include "crow.h"
#include <mariadb/conncpp.hpp>
#include <fstream>
#include <sstream>
#include <memory>
using namespace std;

void setupLoginRoutes(crow::SimpleApp& app) {

    // ========================
    // Serve the login page
    // ========================
    CROW_ROUTE(app, "/").methods("GET"_method)([]() {
        std::ifstream file("frontend/index.html");
        if (!file.is_open())
            return crow::response(404, "index.html not found");

        std::ostringstream buffer;
        buffer << file.rdbuf();
        crow::response res(buffer.str());
        res.set_header("Content-Type", "text/html");
        return res;
    });

    // ========================
    // Login route
    // ========================
    CROW_ROUTE(app, "/login").methods("POST"_method)([](const crow::request& req) {
        auto data = crow::json::load(req.body);
        if (!data || !data.has("id") || !data.has("password") || !data.has("role"))
            return crow::response(400, "Invalid or missing JSON fields");

        string id = data["id"].s();
        string password = data["password"].s();
        string role = data["role"].s();

        if (role != "admins" && role != "members")
            return crow::response(400, "Invalid role");

        try {
            sql::Driver* driver = sql::mariadb::get_driver_instance();
            sql::SQLString url("jdbc:mariadb://localhost:3306/library_system");
            sql::Properties props({{"user", "admin"}, {"password", "admin"}});
            shared_ptr<sql::Connection> conn(driver->connect(url, props));

            string query;
            unique_ptr<sql::PreparedStatement> pstmt;

            if (role == "admins") {
                query = "SELECT admin_id, username FROM admins WHERE admin_id=? AND password=?";
                pstmt = unique_ptr<sql::PreparedStatement>(conn->prepareStatement(query));
                pstmt->setString(1, id);
                pstmt->setString(2, password);
            } else {
                query = "SELECT member_id, name, email, phone, issued_books FROM members WHERE member_id=? AND password=?";
                pstmt = unique_ptr<sql::PreparedStatement>(conn->prepareStatement(query));
                pstmt->setString(1, id);
                pstmt->setString(2, password);
            }

            unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
            crow::json::wvalue result_json;

            if (!res->next()) {
                result_json["status"] = "error";
                result_json["message"] = "Invalid credentials or user not found";
                return crow::response{404, result_json};
            }

            result_json["status"] = "success";
            result_json["role"] = role;

            if (role == "admins") {
                result_json["admin_id"] = res->getString("admin_id");
                result_json["username"] = res->getString("username");
                result_json["message"] = "Admin login successful!";
            } else {
                result_json["member_id"] = res->getString("member_id");
                result_json["name"] = res->getString("name");
                result_json["email"] = res->getString("email");
                result_json["phone"] = res->getString("phone");
                result_json["issued_books"] = res->getInt("issued_books");
                result_json["message"] = "Member login successful!";
            }

            return crow::response{result_json};
        } catch (const sql::SQLException& e) {
            crow::json::wvalue err;
            err["status"] = "error";
            err["message"] = string("Database error: ") + e.what();
            return crow::response{500, err};
        }
    });
}
