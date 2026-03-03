#ifndef WEB_HANDLERS_H
#define WEB_HANDLERS_H

#include <WebServer.h>

// Register all URL routes on the given server instance
void registerRoutes(WebServer& server);

#endif // WEB_HANDLERS_H