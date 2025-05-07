#pragma once

#include <string>
#include <Poco/JSON/Object.h>
#include <Poco/Data/Session.h>
#include <Poco/Data/Statement.h>
#include "../database/DatabaseManager.h"

using namespace Poco::Data::Keywords;
using Poco::Data::Session;

class GovernmentAgency {
private:
    int id = 0;
    std::string agencyName;
    int severityLevel = 0;
    std::string username;
    std::string password;

public:
    GovernmentAgency() = default;

    // Getters & Setters
    int getId() const { return id; }
    std::string getAgencyName() const { return agencyName; }
    int getSeverityLevel() const { return severityLevel; }
    std::string getUsername() const { return username; }
    std::string getPassword() const { return password; }

    void setId(int i) { id = i; }
    void setAgencyName(const std::string& name) { this->agencyName = name; }
    void setSeverityLevel(int level) { severityLevel = level; }
    void setUsername(const std::string& uname) { username = uname; }
    void setPassword(const std::string& pwd) { password = pwd; }

    // JSON serialization
    Poco::JSON::Object::Ptr toJSON() const {
        Poco::JSON::Object::Ptr json = new Poco::JSON::Object();
        json->set("id", id);
        json->set("agencyName", agencyName);
        json->set("severityLevel", severityLevel);
        json->set("username", username);
        return json;
    }

    // Save or update
    bool save() {
        try {
            Session& session = DatabaseManager::getInstance()->getSession();
            std::string insertQuery = "INSERT INTO government_agencies (agency_name, severity_level, username, password) VALUES (?, ?, ?, ?)";
            std::string updateQuery = "UPDATE government_agencies SET agency_name = ?, severity_level = ?, username = ?, password = ? WHERE id = ?";
            std::string lastIdQuery = "SELECT last_insert_rowid()";
            
            if (id == 0) {
                session << insertQuery,
                    use(agencyName), use(severityLevel), use(username), use(password), now;
                Poco::Int64 lastId;
                session << lastIdQuery, into(lastId), now;
                id = static_cast<int>(lastId);
            } else {
                session << updateQuery,
                    use(agencyName), use(severityLevel), use(username), use(password), use(id), now;
            }
            return true;
        } catch (const std::exception& e) {
            std::cerr << "Save error: " << e.what() << std::endl;
            return false;
        }
    }

    static GovernmentAgency findById(int id) {
        GovernmentAgency agency;
        try {
            Session& session = DatabaseManager::getInstance()->getSession();
            session << "SELECT id, agency_name, severity_level, username, password FROM government_agencies WHERE id = ?",
                into(agency.id), into(agency.agencyName), into(agency.severityLevel),
                into(agency.username), into(agency.password), use(id), now;
        } catch (...) {}
        return agency;
    }

    static GovernmentAgency findByUsername(const std::string& uname) {
        GovernmentAgency agency;
        try {
            Session& session = DatabaseManager::getInstance()->getSession();
            std::string username=uname;
            session << "SELECT id, agency_name, severity_level, username, password FROM government_agencies WHERE username = ?",
                into(agency.id), into(agency.agencyName), into(agency.severityLevel),
                into(agency.username), into(agency.password), use(username), now;
        } catch (...) {}
        return agency;
    }

    static bool remove(int id) {
        try {
            Session& session = DatabaseManager::getInstance()->getSession();
            session << "DELETE FROM government_agencies WHERE id = ?", use(id), now;
            return true;
        } catch (...) { return false; }
    }

    static std::vector<GovernmentAgency> findAll() {
        std::vector<GovernmentAgency> list;
        try {
            Session& session = DatabaseManager::getInstance()->getSession();
            Poco::Data::Statement select(session);
            int id, severity; std::string name, uname, pwd;

            select << "SELECT id, agency_name, severity_level, username, password FROM government_agencies",
                into(id), into(name), into(severity), into(uname), into(pwd), range(0, 1);

            while (!select.done()) {
                select.execute();
                GovernmentAgency agency;
                agency.setId(id); agency.setAgencyName(name);
                agency.setSeverityLevel(severity);
                agency.setUsername(uname);
                agency.setPassword(pwd);
                list.push_back(agency);
            }
        } catch (...) {}
        return list;
    }

    bool verifyPassword(const std::string& input) const {
        return input == password; // Replace with hashed comparison for production
    }
    bool offerAid(int requestId, const std::string& aidType) {
        try {
            Session& session = DatabaseManager::getInstance()->getSession();
            std::string description = "Aid provided for request ID: " + std::to_string(requestId);
            std::string aidTypeStr = "Aid";
            session << "UPDATE help_requests SET status = 'Aid Provided' WHERE id = ?", use(requestId), now;
            session << "INSERT INTO alerts (type, message, timestamp, sender) VALUES (?, ?, datetime('now'), ?)",
                use(aidTypeStr), use(description), use(agencyName), now;
            return true;
        } catch (const std::exception& e) {
            std::cerr << "offerAid Error: " << e.what() << std::endl;
            return false;
        }
    }

    std::string showSeverityReport() {
        std::ostringstream report;
        try {
            Session& session = DatabaseManager::getInstance()->getSession();
            Statement select(session);
            int id;
            std::string name;
            int level;
            select << "SELECT id, agency_name, severity_level FROM government_agencies",
                into(id), into(name), into(level), range(0, 1);
            while (!select.done()) {
                select.execute();
                report << "ID: " << id << ", Name: " << name << ", Severity: " << level << "\n";
            }
        } catch (...) {
            report << "Error generating report.";
        }
        return report.str();
    }

    bool allocateResources(const std::string& resourceType, int amount) {
        try {
            std::string resourceT = resourceType;
            Session& session = DatabaseManager::getInstance()->getSession();
            for (int i = 0; i < amount; ++i) {
                session << "INSERT INTO resources (agency_id, resource_name) VALUES (?, ?)",
                    use(id), use(resourceT), now;
            }
            return true;
        } catch (const std::exception& e) {
            std::cerr << "allocateResources Error: " << e.what() << std::endl;
            return false;
        }
    }

    bool emergencyProtocol() {
        try {
            Session& session = DatabaseManager::getInstance()->getSession();
            ++severityLevel;
            std::string emergencyType = "Emergency";
            session << "UPDATE government_agencies SET severity_level = ? WHERE id = ?", use(severityLevel), use(id), now;

            std::string alert = "EMERGENCY PROTOCOL triggered by " + agencyName;
            session << "INSERT INTO alerts (type, message, timestamp, sender) VALUES (?, ?, datetime('now'), ?)",
                use(emergencyType), use(alert), use(agencyName), now;

            return true;
        } catch (const std::exception& e) {
            std::cerr << "emergencyProtocol Error: " << e.what() << std::endl;
            return false;
        }
    }

    std::vector<std::string> trackReliefEffort() {
        std::vector<std::string> results;
        try {
            Session& session = DatabaseManager::getInstance()->getSession();
            Statement select(session);
            int requestId;
            std::string status;
            select << "SELECT id, status FROM help_requests WHERE status = 'Aid Provided'",
                into(requestId), into(status), range(0, 1);
            while (!select.done()) {
                select.execute();
                results.push_back("Request ID: " + std::to_string(requestId) + " => " + status);
            }
        } catch (...) {
            results.push_back("Error tracking efforts.");
        }
        return results;
    }

    // Methods from class diagram
    void trackSeverity() {
        try {
            Session& session = DatabaseManager::getInstance()->getSession();
            session << "INSERT INTO severity_tracking (agency_id, tracking_date) VALUES (?, datetime('now'))",
                use(id), now;
        } catch (const std::exception& e) {
            std::cerr << "Error tracking severity: " << e.what() << std::endl;
        }
    }

    void provisionResources() {
        try {
            Session& session = DatabaseManager::getInstance()->getSession();
            session << "INSERT INTO resource_provision (agency_id, provision_date) VALUES (?, datetime('now'))",
                use(id), now;
        } catch (const std::exception& e) {
            std::cerr << "Error provisioning resources: " << e.what() << std::endl;
        }
    }

    void allocatePersonnel() {
        try {
            Session& session = DatabaseManager::getInstance()->getSession();
            session << "INSERT INTO personnel_allocation (agency_id, allocation_date) VALUES (?, datetime('now'))",
                use(id), now;
        } catch (const std::exception& e) {
            std::cerr << "Error allocating personnel: " << e.what() << std::endl;
        }
    }

    void emergencyBudget() {
        try {
            Session& session = DatabaseManager::getInstance()->getSession();
            session << "INSERT INTO emergency_budget (agency_id, budget_date) VALUES (?, datetime('now'))",
                use(id), now;
        } catch (const std::exception& e) {
            std::cerr << "Error creating emergency budget: " << e.what() << std::endl;
        }
    }

    void callMilitary() {
        try {
            Session& session = DatabaseManager::getInstance()->getSession();
            session << "INSERT INTO military_calls (agency_id, call_date) VALUES (?, datetime('now'))",
                use(id), now;
        } catch (const std::exception& e) {
            std::cerr << "Error calling military: " << e.what() << std::endl;
        }
    }
};
