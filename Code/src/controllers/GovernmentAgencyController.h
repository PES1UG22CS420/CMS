#pragma once

#include <string>
#include <vector>
#include <Poco/JSON/Object.h>
#include "../models/GovernmentAgency.h"
#include "../database/DatabaseManager.h"
#include <Poco/JSON/Array.h>

using namespace Poco::Data::Keywords;
using Poco::Data::Session;

class GovernmentAgencyController {
private:
    DatabaseManager* dbManager;

public:
    GovernmentAgencyController() {
        dbManager = DatabaseManager::getInstance();
    }

    // Register a new agency
    bool registerAgency(const std::string& name, const std::string& username, const std::string& password) {
        try {
            // Check if username already exists
            Session& session = dbManager->getSession();
            Poco::Int64 count = 0;
            std::string usernameCopy = username;  // Create non-const copy
            session << "SELECT COUNT(*) FROM government_agencies WHERE username = ?", 
                into(count), use(usernameCopy), now;
            
            if (count > 0) {
                return false; // Username already exists
            }
            
            // Create new agency
            GovernmentAgency agency;
            agency.setAgencyName(name);
            agency.setUsername(username);
            agency.setPassword(password);
            
            return agency.save();
        } catch (const std::exception& e) {
            std::cerr << "Error registering government agency: " << e.what() << std::endl;
            return false;
        }
    }

    // Login agency
    GovernmentAgency login(const std::string& username, const std::string& password) {
        GovernmentAgency agency = GovernmentAgency::findByUsername(username);
        if (agency.getId() != 0 && agency.verifyPassword(password)) {
            return agency;
        }
        return GovernmentAgency(); // return empty agency if login fails
    }

    // Get agency by ID
    GovernmentAgency getById(int id) {
        return GovernmentAgency::findById(id);
    }

    // Get agency by username
    GovernmentAgency getByUsername(const std::string& username) {
        std::string usernameCopy = username;  // Create non-const copy
        return GovernmentAgency::findByUsername(usernameCopy);
    }

    // Get all agencies
    std::vector<GovernmentAgency> getAll() {
        return GovernmentAgency::findAll();
    }

    // Delete agency by ID
    bool deleteAgency(int id) {
        return GovernmentAgency::remove(id);
    }

    // Update severity level of agency
    bool updateSeverity(int id, int delta) {
        GovernmentAgency agency = GovernmentAgency::findById(id);
        if (agency.getId() == 0) return false;

        int newLevel = agency.getSeverityLevel() + delta;
        if (newLevel < 0) return false; // prevent negative severity

        agency.setSeverityLevel(newLevel);
        return agency.save();
    }

    // Offer aid to a help request
    bool offerAid(int agencyId, int requestId, const std::string& aidType) {
        GovernmentAgency agency = GovernmentAgency::findById(agencyId);
        if (agency.getId() == 0) return false;
        return agency.offerAid(requestId, aidType);
    }

    // Generate a severity report
    std::string getSeverityReport() {
        return GovernmentAgency().showSeverityReport();
    }

    // Allocate resources to agency
    bool allocateResources(int agencyId, const std::string& resourceType, int amount) {
        GovernmentAgency agency = GovernmentAgency::findById(agencyId);
        if (agency.getId() == 0) return false;
        return agency.allocateResources(resourceType, amount);
    }

    // Trigger emergency protocol
    bool triggerEmergency(int agencyId) {
        GovernmentAgency agency = GovernmentAgency::findById(agencyId);
        if (agency.getId() == 0) return false;
        return agency.emergencyProtocol();
    }

    // Track relief efforts
    std::vector<std::string> trackReliefEfforts() {
        return GovernmentAgency().trackReliefEffort();
    }

    bool updateAgencyStatus(int agencyId, const std::string& status) {
        try {
            Session& session = dbManager->getSession();
            std::string statusCopy = status;  // Create non-const copy
            session << "UPDATE government_agencies SET status = ? WHERE id = ?",
                use(statusCopy), use(agencyId), now;
            return true;
        } catch (const std::exception& e) {
            std::cerr << "Error updating agency status: " << e.what() << std::endl;
            return false;
        }
    }

    bool triggerEmergencyProtocol(Poco::JSON::Object::Ptr json) {
        auto session = dbManager->getSession();
        
        try {
            std::string level = json->getValue<std::string>("level");
            std::string description = json->getValue<std::string>("description");
            
            Poco::Data::Statement insert(session);
            insert << "INSERT INTO emergency_protocols (level, description, triggered_at) VALUES (?, ?, NOW())",
                use(level),
                use(description),
                now;
            
            return true;
        } catch (const std::exception& e) {
            std::cerr << "Error triggering emergency protocol: " << e.what() << std::endl;
            return false;
        }
    }

    Poco::JSON::Object trackReliefEffort() {
        auto session = dbManager->getSession();
        Poco::JSON::Object result;
        
        try {
            // Get active operations
            Poco::Data::Statement select(session);
            std::vector<std::string> operations;
            select << "SELECT name, location, status, resources_deployed, personnel_deployed FROM relief_operations WHERE status = 'active'",
                into(operations),
                now;
            
            Poco::JSON::Array operationsArray;
            for (const auto& op : operations) {
                Poco::JSON::Object operation;
                operation.set("name", op);
                operationsArray.add(operation);
            }
            result.set("operations", operationsArray);
            
            // Get metrics
            Poco::Data::Statement metrics(session);
            int activeOps = 0, resources = 0, personnel = 0;
            metrics << "SELECT COUNT(*) as active_ops, "
                   "COALESCE(SUM(resources_deployed), 0) as resources, "
                   "COALESCE(SUM(personnel_deployed), 0) as personnel "
                   "FROM relief_operations WHERE status = 'active'",
                into(activeOps),
                into(resources),
                into(personnel),
                now;
            
            result.set("active_operations", activeOps);
            result.set("resources_deployed", resources);
            result.set("personnel_deployed", personnel);
        } catch (const std::exception& e) {
            std::cerr << "Error tracking relief effort: " << e.what() << std::endl;
        }
        
        return result;
    }

    bool allocatePersonnel(Poco::JSON::Object::Ptr json) {
        auto session = dbManager->getSession();
        
        try {
            std::string type = json->getValue<std::string>("type");
            std::string location = json->getValue<std::string>("location");
            int count = json->getValue<int>("count");
            int priority = json->getValue<int>("priority");
            
            // First, get the active relief operation for this location
            Poco::Data::Statement select(session);
            int operationId = 0;
            select << "SELECT id FROM relief_operations WHERE location = ? AND status = 'active' LIMIT 1",
                into(operationId),
                use(location),
                now;
            
            if (operationId == 0) {
                // Create a new relief operation if none exists
                std::string operationName = type + " Operation";
                Poco::Data::Statement insertOp(session);
                insertOp << "INSERT INTO relief_operations (name, location, status, started_at) "
                        "VALUES (?, ?, 'active', CURRENT_TIMESTAMP)",
                    use(operationName),
                    use(location),
                    now;
                
                // Get the new operation ID
                Poco::Int64 lastId;
                session << "SELECT LAST_INSERT_ID()", into(lastId), now;
                operationId = static_cast<int>(lastId);
            }
            
            // Now insert the personnel allocation
            Poco::Data::Statement insert(session);
            insert << "INSERT INTO personnel_allocations (type, location, count, priority, operation_id, allocated_at, status) "
                   "VALUES (?, ?, ?, ?, ?, CURRENT_TIMESTAMP, 'pending')",
                use(type),
                use(location),
                use(count),
                use(priority),
                use(operationId),
                now;
            
            return true;
        } catch (const std::exception& e) {
            std::cerr << "Error allocating personnel: " << e.what() << std::endl;
            return false;
        }
    }

    Poco::JSON::Object getPersonnelStatus() {
        auto session = dbManager->getSession();
        Poco::JSON::Object result;
        
        try {
            Poco::Data::Statement select(session);
            select << "SELECT pa.type, pa.location, pa.count, pa.priority, pa.status, ro.name as operation_name "
                   "FROM personnel_allocations pa "
                   "LEFT JOIN relief_operations ro ON pa.operation_id = ro.id "
                   "WHERE pa.status != 'completed'";
            
            select.execute();
            Poco::Data::RecordSet rs(select);
            
            Poco::JSON::Array allocationsArray;
            bool more = rs.moveFirst();
            while (more) {
                Poco::JSON::Object allocation;
                allocation.set("type", rs[0].convert<std::string>());
                allocation.set("location", rs[1].convert<std::string>());
                allocation.set("count", rs[2].convert<int>());
                allocation.set("priority", rs[3].convert<int>());
                allocation.set("status", rs[4].convert<std::string>());
                allocation.set("operation", rs[5].convert<std::string>());
                allocationsArray.add(allocation);
                more = rs.moveNext();
            }
            result.set("allocations", allocationsArray);
        } catch (const std::exception& e) {
            std::cerr << "Error getting personnel status: " << e.what() << std::endl;
        }
        
        return result;
    }

    bool createEmergencyBudget(Poco::JSON::Object::Ptr json) {
        auto session = dbManager->getSession();
        
        try {
            std::string category = json->getValue<std::string>("category");
            double amount = json->getValue<double>("amount");
            int priority = json->getValue<int>("priority");
            std::string location = json->getValue<std::string>("location");
            
            // First, get the active relief operation for this location
            Poco::Data::Statement select(session);
            int operationId = 0;
            select << "SELECT id FROM relief_operations WHERE location = ? AND status = 'active' LIMIT 1",
                into(operationId),
                use(location),
                now;
            
            if (operationId == 0) {
                // Create a new relief operation if none exists
                std::string operationName = category + " Operation";
                Poco::Data::Statement insertOp(session);
                insertOp << "INSERT INTO relief_operations (name, location, status) VALUES (?, ?, 'active')",
                    use(operationName),
                    use(location),
                    now;
                
                // Get the new operation ID
                Poco::Int64 lastId;
                session << "SELECT last_insert_rowid()", into(lastId), now;
                operationId = static_cast<int>(lastId);
            }
            
            // Now insert the emergency budget
            Poco::Data::Statement insert(session);
            insert << "INSERT INTO emergency_budgets (category, amount, priority, operation_id, created_at, status) "
                   "VALUES (?, ?, ?, ?, CURRENT_TIMESTAMP, 'available')",
                use(category),
                use(amount),
                use(priority),
                use(operationId),
                now;
            
            return true;
        } catch (const std::exception& e) {
            std::cerr << "Error creating emergency budget: " << e.what() << std::endl;
            return false;
        }
    }

    Poco::JSON::Object getBudgetStatus() {
        auto session = dbManager->getSession();
        Poco::JSON::Object result;
        
        try {
            Poco::Data::Statement select(session);
            select << "SELECT eb.category, eb.amount, eb.priority, eb.status, ro.location, ro.name as operation_name "
                   "FROM emergency_budgets eb "
                   "LEFT JOIN relief_operations ro ON eb.operation_id = ro.id "
                   "WHERE eb.status != 'allocated'";
            
            select.execute();
            Poco::Data::RecordSet rs(select);
            
            Poco::JSON::Array budgetsArray;
            bool more = rs.moveFirst();
            while (more) {
                Poco::JSON::Object budget;
                budget.set("category", rs[0].convert<std::string>());
                budget.set("amount", rs[1].convert<double>());
                budget.set("priority", rs[2].convert<int>());
                budget.set("status", rs[3].convert<std::string>());
                budget.set("location", rs[4].convert<std::string>());
                budget.set("operation", rs[5].convert<std::string>());
                budgetsArray.add(budget);
                more = rs.moveNext();
            }
            result.set("budgets", budgetsArray);
        } catch (const std::exception& e) {
            std::cerr << "Error getting budget status: " << e.what() << std::endl;
        }
        
        return result;
    }

    bool callMilitary(Poco::JSON::Object::Ptr json) {
        auto session = dbManager->getSession();
        
        try {
            std::string type = json->getValue<std::string>("type");
            std::string location = json->getValue<std::string>("location");
            int priority = json->getValue<int>("priority");
            std::string description = json->getValue<std::string>("description");
            
            // First, get the active relief operation for this location
            Poco::Data::Statement select(session);
            int operationId = 0;
            select << "SELECT id FROM relief_operations WHERE location = ? AND status = 'active' LIMIT 1",
                into(operationId),
                use(location),
                now;
            
            if (operationId == 0) {
                // Create a new relief operation if none exists
                std::string operationName = type + " Operation";
                Poco::Data::Statement insertOp(session);
                insertOp << "INSERT INTO relief_operations (name, location, status) VALUES (?, ?, 'active')",
                    use(operationName),
                    use(location),
                    now;
                
                // Get the new operation ID
                Poco::Int64 lastId;
                session << "SELECT last_insert_rowid()", into(lastId), now;
                operationId = static_cast<int>(lastId);
            }
            
            // Now insert the military support request
            Poco::Data::Statement insert(session);
            insert << "INSERT INTO military_support (type, location, priority, description, operation_id, requested_at) "
                   "VALUES (?, ?, ?, ?, ?, NOW())",
                use(type),
                use(location),
                use(priority),
                use(description),
                use(operationId),
                now;
            
            return true;
        } catch (const std::exception& e) {
            std::cerr << "Error calling military support: " << e.what() << std::endl;
            return false;
        }
    }

    Poco::JSON::Object getMilitaryStatus() {
        auto session = dbManager->getSession();
        Poco::JSON::Object result;
        
        try {
            Poco::Data::Statement select(session);
            std::vector<std::string> requests;
            select << "SELECT type, location, priority, description, status FROM military_support WHERE status != 'completed'",
                into(requests),
                now;
            
            Poco::JSON::Array requestsArray;
            for (const auto& req : requests) {
                Poco::JSON::Object request;
                request.set("type", req);
                requestsArray.add(request);
            }
            result.set("requests", requestsArray);
        } catch (const std::exception& e) {
            std::cerr << "Error getting military status: " << e.what() << std::endl;
        }
        
        return result;
    }

    Poco::JSON::Object getEmergencyLevel() {
        auto session = dbManager->getSession();
        Poco::JSON::Object result;
        
        try {
            Poco::Data::Statement select(session);
            std::string level;
            std::string description;
            
            select << "SELECT level, description FROM emergency_protocols WHERE status = 'active' ORDER BY triggered_at DESC LIMIT 1",
                into(level), into(description), now;
            
            if (!level.empty()) {
                result.set("level", level);
                result.set("description", description);
            } else {
                result.set("level", "normal");
                result.set("description", "No active emergency protocols");
            }
        } catch (const std::exception& e) {
            std::cerr << "Error getting emergency level: " << e.what() << std::endl;
            result.set("level", "normal");
            result.set("description", "Error retrieving emergency level");
        }
        
        return result;
    }
};
