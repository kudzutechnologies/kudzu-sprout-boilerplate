#ifndef KUDZUKERNEL_ModuleSDCard_H
#define KUDZUKERNEL_ModuleSDCard_H
#include <Module.hpp>
#include "sdmmc_cmd.h"
#include "esp_vfs_fat.h"

/**
 * Forward declaration of the module singleton
 */
class _ModuleSDCard;
extern _ModuleSDCard ModuleSDCard;

///////////////////////////////////////////
// Declaration of events that can be used
///////////////////////////////////////////

/**
 * An enum that defines the events your modules exchanges
 */
enum ModuleSDCardEvents {
};

///////////////////////////////////////////
// Declaration of the module
///////////////////////////////////////////

/**
 * WiFi module is responsible for activating or de-activating the WiFi on the device
 */
class _ModuleSDCard: public Module {
public:

  _ModuleSDCard();

  /**
   * Return the module configuration
   */
  virtual const ModuleConfig& getModuleConfig();

  /**
   * Write the entire given buffer to the given file
   */
  esp_err_t writeFile(const char * filename, const void * buffer, size_t len);

  /**
   * Append a chunk to the given file
   */
  esp_err_t appendFile(const char * filename, const void * buffer, size_t len);

  /**
   * Read the entire given buffer from the given file
   */
  int readFile(const char * filename, void * buffer, size_t max_len, size_t offset = 0);

  /**
   * Append a line string to the given file
   */
  esp_err_t appendLine(const char * filename, const char * linestr);

  /**
   * Append a line string to the given file
   */
  template< class ...Args >
  esp_err_t appendLinef(const char * filename, const char * linestr, Args && ...args) {
    char buf[256];
    snprintf(buf, 256, linestr, args...);
    return appendFile(filename, buf, strlen(buf));
  }

  /**
   * Return the file size
   */
  esp_err_t getFileSize(const char * filename, size_t * size);

  /**
   * Append a structured record
   */
  template <typename T>
  esp_err_t appendRecord(const char * filename, const T & rec) {
    return appendFile(filename, (void*)&rec, sizeof(T));
  }

  /**
   * Read a structured record from file
   */
  template <typename T>
  esp_err_t readRecord(const char *filename, size_t recno, T * rec) {
    return readFile(filename, (void*)rec, sizeof(T), sizeof(T) * recno);
  }

protected:
  virtual void setup();
  DECLARE_EVENT_HANDLER(all_events);
  virtual std::vector<ValueDefinition> configOptions();
  virtual void configDidSave();
  virtual void activate();
  virtual void deactivate();
private:
  char            __v0003[32];
  bool            __v0002;
  sdmmc_card_t*   __v0004;
  static esp_err_t __v0001(int slot, sdmmc_command_t* cmdinfo);
};


#endif
