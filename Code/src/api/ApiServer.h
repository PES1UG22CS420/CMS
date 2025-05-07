#pragma once

#include <Poco/JSON/Stringifier.h>
#include <Poco/Net/HTTPServer.h>
#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPRequestHandlerFactory.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/Net/ServerSocket.h>
#include <Poco/Util/ServerApplication.h>
#include <Poco/JSON/Object.h>
#include <Poco/JSON/Parser.h>
#include <Poco/Dynamic/Var.h>
#include <Poco/JSON/Array.h>
#include <string>
#include <iostream>

#include "../controllers/PeopleInCrisisController.h"
#include "../controllers/VolunteerController.h"
#include "../controllers/ReliefProviderController.h"
#include "../controllers/GovernmentAgencyController.h"
#include "../controllers/AdminController.h"
#include "../controllers/AlertSystemController.h"
#include "../controllers/HelpRequestController.h"
#include "../database/DatabaseManager.h"

using namespace Poco::Net;
using namespace Poco::Util;
using namespace Poco::JSON;

// Handler for API requests
class ApiRequestHandler : public HTTPRequestHandler {
private:
    PeopleInCrisisController peopleInCrisisController;
    HelpRequestController helpRequestController;
    VolunteerController volunteerController;
    AlertSystemController alertSystemController;
    ReliefProviderController reliefProviderController;
    GovernmentAgencyController governmentAgencyController;
    AdminController adminController;
    AlertSystem alertSystem;

public:
    ApiRequestHandler() : adminController(&alertSystem) {}

    void handleRequest(HTTPServerRequest& request, HTTPServerResponse& response) override {
        response.setContentType("application/json");
        response.add("Access-Control-Allow-Origin", "*");
        response.add("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
        response.add("Access-Control-Allow-Headers", "Content-Type");
        
        if (request.getMethod() == "OPTIONS") {
            response.setStatus(HTTPResponse::HTTP_OK);
            response.send();
            return;
        }
        
        std::string uri = request.getURI();
        
        try {
            if (uri.find("/api/people-in-crisis") == 0) {
                handlePeopleInCrisisRequests(request, response);
            } else if (uri.find("/api/help-requests") == 0) {
                handleHelpRequestsRequests(request, response);
            } else if (uri.find("/api/volunteers") == 0) {
                handleVolunteerRequests(request, response);
            } else if (uri.find("/api/relief-providers") == 0) {
                handleReliefProviderRequests(request, response);
            } else if (uri.find("/api/government-agencies") == 0) {
                handleGovernmentAgencyRequests(request, response);
            } else if (uri.find("/api/alerts/config") == 0) {
                handleAlertConfigRequests(request, response);
            } else if (uri.find("/api/auth") == 0) {
                handleAuthRequests(request, response);
            } else if (uri.find("/api/profiles") == 0) {
                handleProfileRequests(request, response);
            } else if (uri.find("/api/emergency") == 0) {
                handleEmergencyRequests(request, response);
            } else if (uri.find("/api/security") == 0) {
                handleSecurityRequests(request, response);
            } else {
                // Not found
                Object result;
                result.set("status", "error");
                result.set("message", "Endpoint not found");
                response.setStatus(HTTPResponse::HTTP_NOT_FOUND);
                Poco::JSON::Stringifier::stringify(result, response.send());
            }
        } catch (const std::exception& e) {
            // Handle errors
            Object result;
            result.set("status", "error");
            result.set("message", e.what());
            response.setStatus(HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);
            Poco::JSON::Stringifier::stringify(result, response.send());
        }
    }

private:
    // Helper to parse JSON from request body
    Object::Ptr parseJsonBody(HTTPServerRequest& request) {
        std::istream& body = request.stream();
        std::string json;
        char c;
        while (body.get(c)) {
            json += c;
        }
        
        if (json.empty()) {
            return new Object();
        }
        
        Poco::JSON::Parser parser;
        Poco::Dynamic::Var result = parser.parse(json);
        return result.extract<Poco::JSON::Object::Ptr>();
    }
    
    // Handle PeopleInCrisis API endpoints
    void handlePeopleInCrisisRequests(HTTPServerRequest& request, HTTPServerResponse& response) {
        std::string uri = request.getURI();
        std::string method = request.getMethod();
        
        if (method == "GET" && uri == "/api/people-in-crisis") {
            // Get all people in crisis
            auto all = peopleInCrisisController.getAll();
            Array result;
            for (const auto& person : all) {
                result.add(person.toJSON());
            }
            Poco::JSON::Stringifier::stringify(result, response.send());

        } else if (method == "GET" && uri.find("/api/people-in-crisis/") == 0) {
            // Get person by ID
            std::string idStr = uri.substr(22); // Extract ID from URI
            int id = std::stoi(idStr);
            
            auto person = peopleInCrisisController.getById(id);
            if (person.getId() != 0) {
                std::ostringstream oss;
                Poco::JSON::Stringifier::stringify(person.toJSON(), oss);
                response.send() << oss.str();
                
            } else {
                Object result;
                result.set("status", "error");
                result.set("message", "Person not found");
                response.setStatus(HTTPResponse::HTTP_NOT_FOUND);
                Poco::JSON::Stringifier::stringify(result, response.send());
            }
        } else if (method == "POST" && uri == "/api/people-in-crisis/signup") {
            // Create new person in crisis
            auto json = parseJsonBody(request);
            bool success = peopleInCrisisController.signUp(json);
            
            Object result;
            result.set("status", success ? "success" : "error");
            if (!success) {
                result.set("message", "Failed to create user. Username may already exist.");
            }
            Poco::JSON::Stringifier::stringify(result, response.send());
        } else if (method == "PUT" && uri.find("/api/people-in-crisis/") == 0) {
            // Update person
            std::string idStr = uri.substr(22); // Extract ID from URI
            int id = std::stoi(idStr);
            
            auto json = parseJsonBody(request);
            auto person = peopleInCrisisController.getById(id);
            
            if (person.getId() == 0) {
                Object result;
                result.set("status", "error");
                result.set("message", "Person not found");
                response.setStatus(HTTPResponse::HTTP_NOT_FOUND);
                Poco::JSON::Stringifier::stringify(result, response.send());
                return;
            }
            
            if (json->has("status")) {
                std::string status = json->getValue<std::string>("status");
                peopleInCrisisController.updateStatus(id, status);
            }
            
            Poco::JSON::Object result;
            result.set("status", "success");
            Poco::JSON::Stringifier::stringify(result, response.send());
        } else if (method == "DELETE" && uri.find("/api/people-in-crisis/") == 0) {
            // Delete person
            std::string idStr = uri.substr(22); // Extract ID from URI
            int id = std::stoi(idStr);
            
            bool success = peopleInCrisisController.remove(id);
            
            Object result;
            result.set("status", success ? "success" : "error");
            if (!success) {
                result.set("message", "Failed to delete user");
            }
            Poco::JSON::Stringifier::stringify(result, response.send());
        } else {
            // Not found
            Object result;
            result.set("status", "error");
            result.set("message", "Endpoint not found");
            response.setStatus(HTTPResponse::HTTP_NOT_FOUND);
            Poco::JSON::Stringifier::stringify(result, response.send());
        }
    }
    
    // Handle HelpRequests API endpoints
    void handleHelpRequestsRequests(HTTPServerRequest& request, HTTPServerResponse& response) {
        std::string uri = request.getURI();
        std::string method = request.getMethod();
        
        if (method == "GET" && uri == "/api/help-requests") {
            // Get all help requests
            auto all = helpRequestController.getAllRequests();
            Array result;
            for (const auto& req : all) {
                result.add(req.toJSON());
            }
            Poco::JSON::Stringifier::stringify(result, response.send());

        } else if (method == "GET" && uri.find("/api/help-requests/user/") == 0) {
            // Get help requests by user ID
            std::string idStr = uri.substr(24); // Extract ID from URI
            int id = std::stoi(idStr);
            
            auto requests = helpRequestController.getRequestsByRequesterId(id);
            Array result;
            for (const auto& req : requests) {
                result.add(req.toJSON());
            }
            Poco::JSON::Stringifier::stringify(result, response.send());

        } else if (method == "GET" && uri.find("/api/help-requests/") == 0) {
            // Get help request by ID
            std::string idStr = uri.substr(19); // Extract ID from URI
            int id = std::stoi(idStr);
            
            auto request = helpRequestController.getRequestById(id);
            if (request.getId() != 0) {
                std::ostringstream oss;
                Poco::JSON::Stringifier::stringify(request.toJSON(), oss);
                response.send() << oss.str();
            }else {
                Object result;
                result.set("status", "error");
                result.set("message", "Help request not found");
                response.setStatus(HTTPResponse::HTTP_NOT_FOUND);
                Poco::JSON::Stringifier::stringify(result, response.send());

            }
        } else if (method == "POST" && uri == "/api/help-requests") {
            // Create new help request
            auto json = parseJsonBody(request);
            bool success = helpRequestController.createRequest(json);
            
            Object result;
            result.set("status", success ? "success" : "error");
            if (!success) {
                result.set("message", "Failed to create help request");
            }
            Poco::JSON::Stringifier::stringify(result, response.send());

        } else if (method == "PUT" && uri.find("/api/help-requests/") == 0) {
            // Update help request status
            std::string idStr = uri.substr(19); // Extract ID from URI
            int id = std::stoi(idStr);
            
            auto json = parseJsonBody(request);
            std::string status = json->getValue<std::string>("status");
            
            bool success = helpRequestController.updateStatus(id, status);
            
            Object result;
            result.set("status", success ? "success" : "error");
            if (!success) {
                result.set("message", "Failed to update help request");
            }
            Poco::JSON::Stringifier::stringify(result, response.send());

        } else if (method == "DELETE" && uri.find("/api/help-requests/") == 0) {
            // Delete help request
            std::string idStr = uri.substr(19); // Extract ID from URI
            int id = std::stoi(idStr);
            
            bool success = helpRequestController.deleteRequest(id);
            
            Object result;
            result.set("status", success ? "success" : "error");
            if (!success) {
                result.set("message", "Failed to delete help request");
            }
            Poco::JSON::Stringifier::stringify(result, response.send());

        } else {
            // Not found
            Object result;
            result.set("status", "error");
            result.set("message", "Endpoint not found");
            response.setStatus(HTTPResponse::HTTP_NOT_FOUND);
            Poco::JSON::Stringifier::stringify(result, response.send());

        }
    }
    
    // Handle Volunteer API endpoints
    void handleVolunteerRequests(HTTPServerRequest& request, HTTPServerResponse& response) {
        std::string uri = request.getURI();
        std::string method = request.getMethod();
        
        if (method == "GET" && uri == "/api/volunteers") {
            // Get all volunteers
            auto all = volunteerController.getAll();
            Array result;
            for (const auto& volunteer : all) {
                result.add(volunteer.toJSON());
            }
            Poco::JSON::Stringifier::stringify(result, response.send());
        } else if (method == "GET" && uri.find("/api/volunteers/history/") == 0) {
            // Get volunteer history
            std::string idStr = uri.substr(25); // Extract ID from URI
            int id = std::stoi(idStr);
            auto history = volunteerController.getVolunteerHistory(id);
            Array result;
            for (const auto& requestId : history) {
                Object historyItem;
                historyItem.set("requestId", requestId);
                result.add(historyItem);
            }
            Poco::JSON::Stringifier::stringify(result, response.send());
        } else if (method == "GET" && uri.find("/api/volunteers/") == 0) {
            // Get volunteer by ID
            std::string idStr = uri.substr(17); // Extract ID from URI
            int id = std::stoi(idStr);
            auto volunteer = volunteerController.getById(id);
            if (volunteer.getUserID() != 0) {
                std::ostringstream oss;
                Poco::JSON::Stringifier::stringify(volunteer.toJSON(), oss);
                response.send() << oss.str();
            } else {
                Object result;
                result.set("status", "error");
                result.set("message", "Volunteer not found");
                response.setStatus(HTTPResponse::HTTP_NOT_FOUND);
                Poco::JSON::Stringifier::stringify(result, response.send());
            }
        } else if (method == "POST" && uri == "/api/volunteers/signup") {
            // Sign up new volunteer
            auto json = parseJsonBody(request);
            bool success = volunteerController.signUp(json);
            Object result;
            result.set("status", success ? "success" : "error");
            if (!success) {
                result.set("message", "Failed to sign up volunteer.");
            }
            Poco::JSON::Stringifier::stringify(result, response.send());
        } else if (method == "POST" && uri.find("/api/volunteers/donate/") == 0) {
            // Handle donation
            std::string idStr = uri.substr(24); // Extract ID from URI
            int id = std::stoi(idStr);
            auto json = parseJsonBody(request);
            double amount = json->getValue<double>("amount");
            bool success = volunteerController.donate(amount, id);
            Object result;
            result.set("status", success ? "success" : "error");
            if (!success) {
                result.set("message", "Failed to process donation");
            }
            Poco::JSON::Stringifier::stringify(result, response.send());
        } else if (method == "POST" && uri.find("/api/volunteers/help/") == 0) {
            // Handle help on site request
            std::string idStr = uri.substr(22); // Extract ID from URI
            int id = std::stoi(idStr);
            auto json = parseJsonBody(request);
            std::string task = json->getValue<std::string>("task");
            bool success = volunteerController.helpRequest(task, id);
            Object result;
            result.set("status", success ? "success" : "error");
            if (!success) {
                result.set("message", "Failed to register help request");
            }
            Poco::JSON::Stringifier::stringify(result, response.send());
        } else if (method == "PUT" && uri.find("/api/volunteers/") == 0) {
            // Update volunteer availability
            std::string idStr = uri.substr(17); // Extract ID from URI
            int id = std::stoi(idStr);
            auto json = parseJsonBody(request);
            bool available = json->getValue<bool>("available");
            bool success = volunteerController.updateAvailability(id, available);
            Object result;
            result.set("status", success ? "success" : "error");
            if (!success) {
                result.set("message", "Failed to update availability");
            }
            Poco::JSON::Stringifier::stringify(result, response.send());
        } else if (method == "DELETE" && uri.find("/api/volunteers/") == 0) {
            // Delete volunteer
            std::string idStr = uri.substr(17); // Extract ID from URI
            int id = std::stoi(idStr);
            bool success = volunteerController.remove(id);
            Object result;
            result.set("status", success ? "success" : "error");
            if (!success) {
                result.set("message", "Failed to delete volunteer");
            }
            Poco::JSON::Stringifier::stringify(result, response.send());
        } else {
            // Not found
            Object result;
            result.set("status", "error");
            result.set("message", "Endpoint not found");
            response.setStatus(HTTPResponse::HTTP_NOT_FOUND);
            Poco::JSON::Stringifier::stringify(result, response.send());
        }
    }
    
    // Handle Alert Config API endpoints
    void handleAlertConfigRequests(HTTPServerRequest& request, HTTPServerResponse& response) {
        std::string method = request.getMethod();
        if (method == "GET") {
            // Get alert config
            auto config = alertSystemController.getConfig();
            Poco::JSON::Stringifier::stringify(config, response.send());
        } else if (method == "PUT") {
            // Update alert config
            auto json = parseJsonBody(request);
            alertSystemController.updateConfig(json);
            Object result;
            result.set("status", "success");
            Poco::JSON::Stringifier::stringify(result, response.send());
        } else {
            Object result;
            result.set("status", "error");
            result.set("message", "Method not allowed");
            response.setStatus(HTTPResponse::HTTP_METHOD_NOT_ALLOWED);
            Poco::JSON::Stringifier::stringify(result, response.send());
        }
    }
    
    // Handle authentication requests
    void handleAuthRequests(HTTPServerRequest& request, HTTPServerResponse& response) {
        std::string uri = request.getURI();
        std::string method = request.getMethod();
        
        if (method == "POST" && uri == "/api/auth/login") {
            auto json = parseJsonBody(request);
            std::string username = json->getValue<std::string>("username");
            std::string password = json->getValue<std::string>("password");
            std::string userType = json->getValue<std::string>("userType");
            
            bool authenticated = false;
            int userId = 0;
            
            if (userType == "people_in_crisis") {
                authenticated = peopleInCrisisController.authenticate(username, password);
                if (authenticated) {
                    auto person = peopleInCrisisController.getByUsername(username);
                    userId = person.getId();
                }
            } else if (userType == "volunteer") {
                auto volunteer = volunteerController.getByUsername(username);
                if (volunteer.getUserID() != 0 && volunteer.verifyPassword(password)) {
                    authenticated = true;
                    userId = volunteer.getUserID();
                }
            } else if (userType == "relief_provider") {
                auto provider = reliefProviderController.getByUsername(username);
                if (provider.getId() != 0 && provider.verifyPassword(password)) {
                    authenticated = true;
                    userId = provider.getId();
                }
            } else if (userType == "government_agency") {
                auto agency = governmentAgencyController.getByUsername(username);
                if (agency.getId() != 0 && agency.verifyPassword(password)) {
                    authenticated = true;
                    userId = agency.getId();
                }
            } else if (userType == "admin") {
                auto admin = adminController.login(username, password);
                if (admin.getId() != 0) {
                    authenticated = true;
                    userId = admin.getId();
                }
            }
            
            Object result;
            if (authenticated) {
                result.set("status", "success");
                result.set("userId", userId);
                result.set("userType", userType);
            } else {
                result.set("status", "error");
                result.set("message", "Invalid credentials");
            }
            
            Poco::JSON::Stringifier::stringify(result, response.send());
        } else {
            // Not found
            Object result;
            result.set("status", "error");
            result.set("message", "Endpoint not found");
            response.setStatus(HTTPResponse::HTTP_NOT_FOUND);
            Poco::JSON::Stringifier::stringify(result, response.send());
        }
    }

    // Handle ReliefProvider API endpoints
    void handleReliefProviderRequests(HTTPServerRequest& request, HTTPServerResponse& response) {
        std::string uri = request.getURI();
        std::string method = request.getMethod();
        
        if (method == "POST" && uri == "/api/relief-providers/signup") {
            // Sign up new relief provider
            auto json = parseJsonBody(request);
            bool success = reliefProviderController.signUp(json);
            Object result;
            result.set("status", success ? "success" : "error");
            if (!success) {
                result.set("message", "Failed to sign up relief provider.");
            }
            Poco::JSON::Stringifier::stringify(result, response.send());
        } else if (method == "GET" && uri == "/api/relief-providers") {
            // Get all relief providers
            auto all = reliefProviderController.getAll();
            Array result;
            for (const auto& provider : all) {
                result.add(provider.toJSON());
            }
            Poco::JSON::Stringifier::stringify(result, response.send());
        } else if (method == "GET" && uri.find("/api/relief-providers/") == 0) {
            // Get relief provider by ID
            std::string idStr = uri.substr(22); // Extract ID from URI
            int id = std::stoi(idStr);
            auto provider = reliefProviderController.getById(id);
            if (provider.getId() != 0) {
                Poco::JSON::Stringifier::stringify(provider.toJSON(), response.send());
            } else {
                Object result;
                result.set("status", "error");
                result.set("message", "Relief provider not found");
                response.setStatus(HTTPResponse::HTTP_NOT_FOUND);
                Poco::JSON::Stringifier::stringify(result, response.send());
            }
        } else {
            // Not found
            Object result;
            result.set("status", "error");
            result.set("message", "Endpoint not found");
            response.setStatus(HTTPResponse::HTTP_NOT_FOUND);
            Poco::JSON::Stringifier::stringify(result, response.send());
        }
    }

    // Handle GovernmentAgency API endpoints
    void handleGovernmentAgencyRequests(HTTPServerRequest& request, HTTPServerResponse& response) {
        std::string uri = request.getURI();
        std::string method = request.getMethod();
        
        if (method == "POST" && uri == "/api/government-agencies/signup") {
            // Sign up new government agency
            auto json = parseJsonBody(request);
            bool success = governmentAgencyController.registerAgency(
                json->getValue<std::string>("name"),
                json->getValue<std::string>("username"),
                json->getValue<std::string>("password")
            );
            Object result;
            result.set("status", success ? "success" : "error");
            if (!success) {
                result.set("message", "Failed to sign up government agency.");
            }
            Poco::JSON::Stringifier::stringify(result, response.send());
        } else if (method == "GET" && uri == "/api/government-agencies") {
            // Get all government agencies
            auto all = governmentAgencyController.getAll();
            Array result;
            for (const auto& agency : all) {
                result.add(agency.toJSON());
            }
            Poco::JSON::Stringifier::stringify(result, response.send());
        } else if (method == "GET" && uri.find("/api/government-agencies/") == 0) {
            // Get government agency by ID
            std::string idStr = uri.substr(25); // Extract ID from URI
            int id = std::stoi(idStr);
            auto agency = governmentAgencyController.getById(id);
            if (agency.getId() != 0) {
                Poco::JSON::Stringifier::stringify(agency.toJSON(), response.send());
            } else {
                Object result;
                result.set("status", "error");
                result.set("message", "Government agency not found");
                response.setStatus(HTTPResponse::HTTP_NOT_FOUND);
                Poco::JSON::Stringifier::stringify(result, response.send());
            }
        } else {
            // Not found
            Object result;
            result.set("status", "error");
            result.set("message", "Endpoint not found");
            response.setStatus(HTTPResponse::HTTP_NOT_FOUND);
            Poco::JSON::Stringifier::stringify(result, response.send());
        }
    }

    // Handle Profile API endpoints
    void handleProfileRequests(HTTPServerRequest& request, HTTPServerResponse& response) {
        std::string uri = request.getURI();
        std::string method = request.getMethod();
        
        if (method == "GET" && uri.find("/api/profiles/") == 0) {
            // Get profile by user ID
            std::string idStr = uri.substr(14); // Extract ID from URI
            int id = std::stoi(idStr);
            
            auto profile = peopleInCrisisController.getProfile(id);
            if (profile.getId() != 0) {
                Poco::JSON::Stringifier::stringify(profile.toJSON(), response.send());
            } else {
                Object result;
                result.set("status", "error");
                result.set("message", "Profile not found");
                response.setStatus(HTTPResponse::HTTP_NOT_FOUND);
                Poco::JSON::Stringifier::stringify(result, response.send());
            }
        } else if (method == "PUT" && uri.find("/api/profiles/") == 0) {
            // Update profile
            std::string idStr = uri.substr(14); // Extract ID from URI
            int id = std::stoi(idStr);
            
            auto json = parseJsonBody(request);
            bool success = peopleInCrisisController.updateProfile(id, json);
            
            Object result;
            result.set("status", success ? "success" : "error");
            if (!success) {
                result.set("message", "Failed to update profile");
            }
            Poco::JSON::Stringifier::stringify(result, response.send());
        }
    }

    // Handle Emergency API endpoints
    void handleEmergencyRequests(HTTPServerRequest& request, HTTPServerResponse& response) {
        std::string uri = request.getURI();
        std::string method = request.getMethod();
        
        if (method == "GET" && uri == "/api/emergency/level") {
            // Get current emergency level
            auto level = governmentAgencyController.getEmergencyLevel();
            Poco::JSON::Stringifier::stringify(level, response.send());
        } else if (method == "POST" && uri == "/api/emergency/protocol") {
            // Trigger emergency protocol
            auto json = parseJsonBody(request);
            bool success = governmentAgencyController.triggerEmergencyProtocol(json);
            
            Object result;
            result.set("status", success ? "success" : "error");
            if (!success) {
                result.set("message", "Failed to trigger emergency protocol");
            }
            Poco::JSON::Stringifier::stringify(result, response.send());
        } else if (method == "GET" && uri == "/api/emergency/relief-effort") {
            // Get relief effort tracking
            auto data = governmentAgencyController.trackReliefEffort();
            Poco::JSON::Stringifier::stringify(data, response.send());
        } else if (method == "POST" && uri == "/api/emergency/personnel") {
            // Allocate personnel
            auto json = parseJsonBody(request);
            bool success = governmentAgencyController.allocatePersonnel(json);
            
            Object result;
            result.set("status", success ? "success" : "error");
            if (!success) {
                result.set("message", "Failed to allocate personnel");
            }
            Poco::JSON::Stringifier::stringify(result, response.send());
        } else if (method == "GET" && uri == "/api/emergency/personnel") {
            // Get personnel status
            auto status = governmentAgencyController.getPersonnelStatus();
            Poco::JSON::Stringifier::stringify(status, response.send());
        } else if (method == "POST" && uri == "/api/emergency/budget") {
            // Create emergency budget
            auto json = parseJsonBody(request);
            bool success = governmentAgencyController.createEmergencyBudget(json);
            
            Object result;
            result.set("status", success ? "success" : "error");
            if (!success) {
                result.set("message", "Failed to create emergency budget");
            }
            Poco::JSON::Stringifier::stringify(result, response.send());
        } else if (method == "GET" && uri == "/api/emergency/budget") {
            // Get budget status
            auto status = governmentAgencyController.getBudgetStatus();
            Poco::JSON::Stringifier::stringify(status, response.send());
        } else if (method == "POST" && uri == "/api/emergency/military") {
            // Request military support
            auto json = parseJsonBody(request);
            bool success = governmentAgencyController.callMilitary(json);
            
            Object result;
            result.set("status", success ? "success" : "error");
            if (!success) {
                result.set("message", "Failed to request military support");
            }
            Poco::JSON::Stringifier::stringify(result, response.send());
        } else if (method == "GET" && uri == "/api/emergency/military") {
            // Get military support status
            auto status = governmentAgencyController.getMilitaryStatus();
            Poco::JSON::Stringifier::stringify(status, response.send());
        } else {
            response.setStatus(HTTPResponse::HTTP_NOT_FOUND);
            Object result;
            result.set("status", "error");
            result.set("message", "Endpoint not found");
            Poco::JSON::Stringifier::stringify(result, response.send());
        }
    }

    // Handle Security API endpoints
    void handleSecurityRequests(HTTPServerRequest& request, HTTPServerResponse& response) {
        std::string uri = request.getURI();
        std::string method = request.getMethod();
        
        if (method == "GET" && uri == "/api/security/logs") {
            // Get security logs
            auto logs = adminController.getSecurityLogs();
            Poco::JSON::Stringifier::stringify(logs, response.send());
        } else if (method == "PUT" && uri == "/api/security/settings") {
            // Update security settings
            auto json = parseJsonBody(request);
            bool success = adminController.updateSecuritySettings(json);
            
            Object result;
            result.set("status", success ? "success" : "error");
            if (!success) {
                result.set("message", "Failed to update security settings");
            }
            Poco::JSON::Stringifier::stringify(result, response.send());
        } else if (method == "GET" && uri == "/api/security/status") {
            // Get security status
            auto status = adminController.getSecurityStatus();
            Poco::JSON::Stringifier::stringify(status, response.send());
        } else if (method == "GET" && uri == "/api/security/verifications") {
            // Get pending verifications
            auto verifications = adminController.getPendingVerifications();
            Poco::JSON::Stringifier::stringify(verifications, response.send());
        } else if (method == "POST" && uri.find("/api/security/verify/") == 0) {
            // Verify account
            std::string idStr = uri.substr(21); // Extract ID from URI
            int id = std::stoi(idStr);
            
            auto json = parseJsonBody(request);
            bool approved = json->getValue<std::string>("status") == "approved";
            std::string notes = json->getValue<std::string>("notes");
            
            bool success = adminController.verifyAccount(id, approved, notes);
            
            Object result;
            result.set("status", success ? "success" : "error");
            if (!success) {
                result.set("message", "Failed to verify account");
            }
            Poco::JSON::Stringifier::stringify(result, response.send());
        }
    }
};

// Factory for creating request handlers
class ApiRequestHandlerFactory : public HTTPRequestHandlerFactory {
public:
    ApiRequestHandlerFactory() {
        // Initialize database
        DatabaseManager::getInstance();
    }

    HTTPRequestHandler* createRequestHandler(const HTTPServerRequest&) override {
        return new ApiRequestHandler();
    }
};

// Main server application
class ApiServer : public ServerApplication {
protected:
    int main(const std::vector<std::string>&) override {
        HTTPServerParams* params = new HTTPServerParams;
        params->setMaxQueued(100);
        params->setMaxThreads(16);
        
        ServerSocket socket(8080); // Listen on port 8080
        HTTPServer server(new ApiRequestHandlerFactory(), socket, params);
        
        server.start();
        std::cout << "Server started on port 8080" << std::endl;
        
        waitForTerminationRequest();
        
        std::cout << "Shutting down..." << std::endl;
        server.stop();
        
        return Application::EXIT_OK;
    }
};