#pragma once

#include <string>
#include <Poco/JSON/Object.h>
#include "../models/AlertSystem.h"

// Controller for AlertSystem
class AlertSystemController {
private:
    AlertSystem model;

public:
    // Constructor
    AlertSystemController() {}

    // Methods from class diagram
    bool registerSubscriber(const std::string& subscriber) {
        model.addSubscriber(subscriber);
        return true;
    }
    
    bool unregisterSubscriber(const std::string& subscriber) {
        model.removeSubscriber(subscriber);
        return true;
    }
    
    bool updateAlertMessage(const std::string& message) {
        model.broadcastAlertMessage(message);
        return true;
    }
    
    // CRUD operations for the model
    AlertSystem getModel() {
        return model;
    }
    
    std::vector<std::string> getAllSubscribers() {
        const std::list<std::string>& subscribers = model.getSubscribers();
        return std::vector<std::string>(subscribers.begin(), subscribers.end());
    }

    // Config methods
    Poco::JSON::Object::Ptr getConfig() {
        return model.toJSON();
    }
    void updateConfig(const Poco::JSON::Object::Ptr& json) {
        if (json->has("urgency_threshold")) {
            model.setUrgencyThreshold(json->getValue<int>("urgency_threshold"));
        }
        if (json->has("auto_assign")) {
            model.setAutoAssign(json->getValue<bool>("auto_assign"));
        }
    }
};