// NB-IoT modem driver (SIM7020E-style AT interface) - HTTP(S) POST of a JSON body.
//
// NOTE: written from the SIM7020 AT command manual / application note and NOT yet
// tested against real hardware. Lines marked VERIFY should be checked against the
// AT manual of your exact module (and operator) before relying on them.
#pragma once
#include <stdint.h>

class NbIotModem {
public:
    // idle: called repeatedly while waiting for the modem (keep the GPS UART drained)
    void begin(void (*idle)());
    bool init();                               // AT sync, SIM check, PSM/eDRX off, APN
    bool attached();                           // registered on the NB-IoT network?
    bool waitAttached(uint32_t timeoutMs);
    // POST JSON. Returns the HTTP status code (e.g. 200), or:
    //  -1 modem/connection error, -2 payload too long, -3 no response before timeout
    int  postJson(const char* host, uint16_t port, bool https, const char* path, const char* json);
    int  signalQuality();                      // AT+CSQ (0..31, 99 = unknown)

private:
    int  waitFor(const char* token, uint32_t timeoutMs);
    bool cmd(const char* c, const char* expect = "OK", uint32_t timeoutMs = 2000);
    void flushInput();
    void idleWait(uint32_t ms);
    void (*idle_)() = nullptr;
};
