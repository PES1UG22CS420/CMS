#pragma once

#include <string>
#include <vector>
#include <Poco/JSON/Object.h>
#include "../models/HelpRequest.h"
#include "../models/PeopleInCrisis.h"
#include "../database/DatabaseManager.h"

// Controller for HelpRequest
class HelpRequestController {
public:
    // Constructor
    HelpRequestController() {}

    // Create a new help request
    bool createRequest(const Poco::JSON::Object::Ptr& data) {
        try {
            int requesterId = data->getValue<int>("requesterId");
            std::string type = data->getValue<std::string>("type");
            std::string description = data->getValue<std::string>("description");
            std::string location = data->getValue<std::string>("location");
            int urgency = data->getValue<int>("urgency");
            
            // Create new request
            HelpRequest request;
            request.setRequesterId(requesterId);
            request.setType(type);
            request.setDescription(description);
            request.setLocation(location);
            request.setUrgency(urgency);
            request.setStatus("Pending");
            
            // Save the request
            bool success = request.save();
            
            // Update the person's status
            if (success) {
                PeopleInCrisis person = PeopleInCrisis::findById(requesterId);
                if (person.getId() != 0) {
                    person.setHasActiveRequest(true);
                    person.save();
                }
            }
            
            return success;
        } catch (const std::exception& e) {
            std::cerr << "Error creating help request: " << e.what() << std::endl;
            return false;
        }
    }
    
    // Update a help request status
    bool updateStatus(int requestId, const std::string& status) {
        try {
            HelpRequest request = HelpRequest::findById(requestId);
            if (request.getId() == 0) {
                return false;
            }
            
            request.setStatus(status);
            bool success = request.save();
            
            // If status is "Resolved" or "Cancelled", update the person's status
            if (success && (status == "Resolved" || status == "Cancelled")) {
                PeopleInCrisis person = PeopleInCrisis::findById(request.getRequesterId());
                if (person.getId() != 0) {
                    person.setHasActiveRequest(false);
                    person.save();
                }
            }
            
            return success;
        } catch (const std::exception& e) {
            std::cerr << "Error updating help request status: " << e.what() << std::endl;
            return false;
        }
    }
    
    // Get all help requests
    std::vector<HelpRequest> getAllRequests() {
        return HelpRequest::findAll();
    }
    
    // Get help requests by requester ID
    std::vector<HelpRequest> getRequestsByRequesterId(int requesterId) {
        return HelpRequest::findByRequesterId(requesterId);
    }
    
    // Get a help request by ID
    HelpRequest getRequestById(int requestId) {
        return HelpRequest::findById(requestId);
    }
    
    // Delete a help request
    bool deleteRequest(int requestId) {
        return HelpRequest::remove(requestId);
    }
};