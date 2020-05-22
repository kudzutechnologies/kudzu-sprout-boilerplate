#ifndef KUDZUKERNEL_MODULEHTTPSERVER_H
#define KUDZUKERNEL_MODULEHTTPSERVER_H
#include <Module.hpp>
#include <esp_http_server.h>
#include <vector>

#define MODULE_HTTPSERVER_MAX_ROUTES  24

enum ModuleHTTPServerEvents {
  /**
   * [OUT] Called when the HTTP server is ready
   */
  EVENT_HTTPSERVER_READY,

  /**
   * [OUT] Called when the HTTP server is ready and the listeners should
   * dispatch the REGISTER_ROUTE event
   */
  EVENT_HTTPSERVER_REGISTER_ROUTES,

  /**
   * [IN] Sent when the user wants to register an event handler
   *
   * @argument httpd_uri_t
   */
  EVENT_HTTPSERVER_REGISTER_ROUTE,
};

/**
 * The HTTP server serves arbitrary URLs over the HTTP port
 */
class _ModuleHTTPServer: public Module {
public:

  _ModuleHTTPServer();

  /**
   * Return the module configuration
   */
  virtual const ModuleConfig& getModuleConfig();

  /**
   * Register a route
   */
  esp_err_t addRoute(const httpd_uri_t route);

  /**
   * Unregister a previously registered route
   */
  esp_err_t removeRoute(const char *uri, httpd_method_t method);

protected:
  virtual void setup();
  DECLARE_EVENT_HANDLER(all_events);
  DECLARE_EVENT_HANDLER(mm_events);
  virtual void activate();
  virtual void deactivate();
private:
  httpd_handle_t  __v0004;
  httpd_uri_t     __v0003[MODULE_HTTPSERVER_MAX_ROUTES];
  uint8_t         __v0001;
  bool            __v0002;
};

extern _ModuleHTTPServer ModuleHTTPServer;

#endif
