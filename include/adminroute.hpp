#pragma once
#include "crow.h"
#include <mariadb/conncpp.hpp>
#include <memory>

void setupAdminRoutes(crow::SimpleApp& app, std::shared_ptr<sql::Connection> conn);
