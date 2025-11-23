#include "crow.h"
#include "../include/routes_login.hpp"
#include "routes/admin_routes.cpp"
#include "../include/adminroute.hpp"
#include "../include/memberroute.hpp"
#include "routes/member_routes.cpp"

int main() {
    crow::SimpleApp app;

    setupLoginRoutes(app);  // all login stuff handled here
    
    setupAdminRoutes(app);

    setupMemberRoutes(app);
     
    app.port(80).multithreaded().run();
}
