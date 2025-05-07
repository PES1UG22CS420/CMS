#include "api/ApiServer.h"

int main(int argc, char** argv) {
    ApiServer app;
    return app.run(argc, argv);
}