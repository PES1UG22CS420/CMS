#pragma once

#include <string>
#include <memory>

// Command interface
class HelpRequestCommand {
public:
    virtual ~HelpRequestCommand() = default;
    virtual bool execute() = 0;
    virtual void undo() = 0;
};

// Concrete commands
class CreateHelpRequestCommand : public HelpRequestCommand {
private:
    int requesterId;
    std::string type;
    std::string description;
    std::string location;
    int urgency;

public:
    CreateHelpRequestCommand(int requesterId, const std::string& type, 
                           const std::string& description, const std::string& location, 
                           int urgency)
        : requesterId(requesterId), type(type), description(description), 
          location(location), urgency(urgency) {}

    bool execute() override {
        // Implementation using HelpRequestController
        return true;
    }

    void undo() override {
        // Implementation to undo the request creation
    }
};

class UpdateHelpRequestCommand : public HelpRequestCommand {
private:
    int requestId;
    std::string newStatus;

public:
    UpdateHelpRequestCommand(int requestId, const std::string& newStatus)
        : requestId(requestId), newStatus(newStatus) {}

    bool execute() override {
        // Implementation using HelpRequestController
        return true;
    }

    void undo() override {
        // Implementation to undo the status update
    }
};

// Command invoker
class HelpRequestInvoker {
private:
    std::vector<std::shared_ptr<HelpRequestCommand>> commandHistory;

public:
    void executeCommand(std::shared_ptr<HelpRequestCommand> command) {
        if (command->execute()) {
            commandHistory.push_back(command);
        }
    }

    void undoLastCommand() {
        if (!commandHistory.empty()) {
            commandHistory.back()->undo();
            commandHistory.pop_back();
        }
    }
}; 