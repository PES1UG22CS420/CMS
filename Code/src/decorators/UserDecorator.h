#pragma once

#include <string>
#include <memory>

// Component interface
class User {
public:
    virtual ~User() = default;
    virtual std::string getType() const = 0;
    virtual bool hasPermission(const std::string& permission) const = 0;
};

// Concrete component
class BaseUser : public User {
private:
    std::string type;

public:
    BaseUser(const std::string& type) : type(type) {}

    std::string getType() const override {
        return type;
    }

    bool hasPermission(const std::string& permission) const override {
        return false;
    }
};

// Decorator base class
class UserDecorator : public User {
protected:
    std::shared_ptr<User> user;

public:
    UserDecorator(std::shared_ptr<User> user) : user(user) {}

    std::string getType() const override {
        return user->getType();
    }

    bool hasPermission(const std::string& permission) const override {
        return user->hasPermission(permission);
    }
};

// Concrete decorators
class AdminDecorator : public UserDecorator {
public:
    AdminDecorator(std::shared_ptr<User> user) : UserDecorator(user) {}

    bool hasPermission(const std::string& permission) const override {
        return true; // Admins have all permissions
    }
};

class VolunteerDecorator : public UserDecorator {
public:
    VolunteerDecorator(std::shared_ptr<User> user) : UserDecorator(user) {}

    bool hasPermission(const std::string& permission) const override {
        if (permission == "help_request" || permission == "donate") {
            return true;
        }
        return user->hasPermission(permission);
    }
};

class ReliefProviderDecorator : public UserDecorator {
public:
    ReliefProviderDecorator(std::shared_ptr<User> user) : UserDecorator(user) {}

    bool hasPermission(const std::string& permission) const override {
        if (permission == "provide_aid" || permission == "request_resources") {
            return true;
        }
        return user->hasPermission(permission);
    }
}; 