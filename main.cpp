#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>
#include <crow.h>
#include <optional>
#include <unordered_map>

using json = nlohmann::json;

std::optional<json> load_users(const std::string& filepath) {
    try {
        std::ifstream users_file(filepath);
        if (!users_file.is_open()) {
            return std::nullopt;
        }
        json users;
        users_file >> users;
        return users;
    } catch (const std::exception& e) {
        std::cerr << "Error loading JSON: " << e.what() << std::endl;
        return std::nullopt;
    }
}

std::optional<std::unordered_map<std::string, std::string>> authenticate_user(
    const json& users, 
    const std::string& username, 
    const std::string& password
) {
    if (!users.contains("authors")) {
        return std::nullopt;
    }

    for (const auto& user : users["authors"]) {
        if (user["user"] == username && user["password"] == password) {
            return std::unordered_map<std::string, std::string>{
                {"user", username}
            };
        }
    }
    return std::nullopt;
}

int main() {
    crow::SimpleApp app;

    CROW_ROUTE(app, "/watan/ilovecats")
        .methods("GET"_method)
        ([](const crow::request& req) {
            auto user_p = req.url_params.get("user");
            auto pass_p = req.url_params.get("password");

            if (!user_p || !pass_p) {
                return crow::response(400, "Missing parameters");
            }

            auto users = load_users("db/users.json");
            if (!users) {
                return crow::response(500, "Error loading user database");
            }

            auto authenticated_user = authenticate_user(*users, user_p, pass_p);

            if (authenticated_user) {
                crow::json::wvalue response_json;
                response_json["status"] = "success";
                response_json["message"] = "Authentication successful";
                response_json["user"] = authenticated_user->at("user");
                return crow::response(200, response_json);
            } else {
                return crow::response(401, "Authentication failed");
            }
        });

    uint16_t port = 8080;
    app.port(port).multithreaded().run();
    return 0;
}