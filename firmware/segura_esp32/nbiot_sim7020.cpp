#include <Arduino.h>
#include <string.h>
#include "config.h"
#include "nbiot_sim7020.h"

static HardwareSerial modemSerial(1);
static char rx[2048];
static size_t rxLen = 0;

void NbIotModem::begin(void (*idle)()) {
    idle_ = idle;
    modemSerial.setRxBufferSize(2048);
    modemSerial.begin(MODEM_BAUD, SERIAL_8N1, PIN_MODEM_RX, PIN_MODEM_TX);
#if PIN_MODEM_PWRKEY >= 0
    pinMode(PIN_MODEM_PWRKEY, OUTPUT);
    digitalWrite(PIN_MODEM_PWRKEY, LOW);  delay(100);      // VERIFY pulse polarity/length for your board
    digitalWrite(PIN_MODEM_PWRKEY, HIGH); delay(1200);
    digitalWrite(PIN_MODEM_PWRKEY, LOW);
#endif
}

void NbIotModem::idleWait(uint32_t ms) {
    uint32_t t = millis();
    while (millis() - t < ms) { if (idle_) idle_(); delay(2); }
}

void NbIotModem::flushInput() { while (modemSerial.available()) modemSerial.read(); }

// Collect modem output until `token` appears, an ERROR appears, or the timeout expires.
// Returns 1 = token seen, -1 = ERROR seen, 0 = timeout.
int NbIotModem::waitFor(const char* token, uint32_t timeoutMs) {
    rxLen = 0; rx[0] = 0;
    uint32_t t0 = millis();
    while (millis() - t0 < timeoutMs) {
        while (modemSerial.available() && rxLen < sizeof(rx) - 1) { rx[rxLen++] = (char)modemSerial.read(); rx[rxLen] = 0; }
        if (strstr(rx, token)) return 1;
        if (strstr(rx, "ERROR")) return -1;
        if (idle_) idle_();
        delay(2);
    }
    return 0;
}

bool NbIotModem::cmd(const char* c, const char* expect, uint32_t timeoutMs) {
    flushInput();
    modemSerial.print(c); modemSerial.print("\r\n");
    return waitFor(expect, timeoutMs) == 1;
}

bool NbIotModem::init() {
    bool alive = false;
    for (int i = 0; i < 20 && !alive; i++) { alive = cmd("AT", "OK", 500); if (!alive) idleWait(500); }
    if (!alive) { Serial.println("[NB] modem not responding"); return false; }

    cmd("ATE0");
    cmd("AT+CMEE=2");                                   // verbose error codes
    if (!cmd("AT+CPIN?", "READY", 5000)) { Serial.println("[NB] SIM not ready"); return false; }
    cmd("AT+CPSMS=0");                                  // power-saving mode off: stay reachable, lowest latency
    cmd("AT+CEDRXS=0,5");                               // eDRX off (ignore if unsupported)

    // Default bearer APN (SIMCom app note: CFUN=0 -> set APN -> CFUN=1)   VERIFY
    char c[96];
    cmd("AT+CFUN=0", "OK", 10000);
    snprintf(c, sizeof c, "AT*MCGDEFCONT=\"IP\",\"%s\"", NBIOT_APN);
    if (!cmd(c, "OK", 3000)) Serial.println("[NB] APN command rejected (check AT*MCGDEFCONT for your module)");
    cmd("AT+CFUN=1", "OK", 10000);
    idleWait(2000);
    return true;
}

bool NbIotModem::attached() {
    // +CGREG: <n>,<stat>  stat 1 = home, 5 = roaming        VERIFY (some firmwares prefer +CEREG)
    if (!cmd("AT+CGREG?", "OK", 2000)) return false;
    const char* p = strstr(rx, "+CGREG:");
    if (!p) return false;
    p = strchr(p, ','); if (!p) return false;
    int stat = atoi(p + 1);
    return stat == 1 || stat == 5;
}

bool NbIotModem::waitAttached(uint32_t timeoutMs) {
    uint32_t t0 = millis();
    while (millis() - t0 < timeoutMs) {
        if (attached()) { Serial.printf("[NB] attached (CSQ=%d)\n", signalQuality()); return true; }
        idleWait(3000);
    }
    return false;
}

int NbIotModem::signalQuality() {
    if (!cmd("AT+CSQ", "OK", 1000)) return 99;
    const char* p = strstr(rx, "+CSQ:");
    return p ? atoi(p + 5) : 99;
}

static const char HEX_DIGITS[] = "0123456789ABCDEF";

int NbIotModem::postJson(const char* host, uint16_t port, bool https, const char* path, const char* json)
{
    const size_t jl = strlen(json);
    if (jl > 440) return -2;                            // keeps the AT command line within typical limits   VERIFY

    char url[128];
    snprintf(url, sizeof url, "%s://%s:%u/", https ? "https" : "http", host, (unsigned)port);

    cmd("AT+CHTTPDESTROY=0", "OK", 2000);               // clear any stale instance (error is fine)
    char c[1100];
    snprintf(c, sizeof c, "AT+CHTTPCREATE=\"%s\"", url);
    if (!cmd(c, "OK", 10000)) return -1;
    const char* p = strstr(rx, "+CHTTPCREATE:");
    int id = p ? atoi(p + 13) : 0;

    snprintf(c, sizeof c, "AT+CHTTPCON=%d", id);        // TCP (+TLS) connect
    if (!cmd(c, "OK", 60000)) { snprintf(c, sizeof c, "AT+CHTTPDESTROY=%d", id); cmd(c); return -1; }
    idleWait(500);

    // AT+CHTTPSEND=<id>,<method 1=POST>,"<path>","<hex header>","<content-type>","<hex body>"   VERIFY
    int n = snprintf(c, sizeof c, "AT+CHTTPSEND=%d,1,\"%s\",\"\",\"application/json\",\"", id, path);
    for (size_t i = 0; i < jl && n < (int)sizeof c - 4; i++) {
        c[n++] = HEX_DIGITS[((uint8_t)json[i]) >> 4];
        c[n++] = HEX_DIGITS[((uint8_t)json[i]) & 0x0F];
    }
    c[n++] = '"'; c[n] = 0;

    flushInput();
    modemSerial.print(c); modemSerial.print("\r\n");

    // The HTTP response arrives as an unsolicited  +CHTTPNMIC: <id>,<flag>,<total>,<len>,<hex data>
    int status = -3;
    int w = waitFor("+CHTTPNMIC:", HTTP_TIMEOUT_MS);
    if (w == -1) status = -1;
    if (w == 1) {
        idleWait(300);                                  // let the rest of the URC arrive
        while (modemSerial.available() && rxLen < sizeof(rx) - 1) { rx[rxLen++] = (char)modemSerial.read(); rx[rxLen] = 0; }
        const char* q = strstr(rx, "+CHTTPNMIC:");
        // skip to the 5th field (hex data) and decode the first bytes: "HTTP/1.1 200 ..."
        int commas = 0; while (q && *q && commas < 4) { if (*q == ',') commas++; q++; }
        char text[40]; int tl = 0;
        while (q && isxdigit((unsigned char)q[0]) && isxdigit((unsigned char)q[1]) && tl < (int)sizeof text - 1) {
            auto hv = [](char ch) { return ch <= '9' ? ch - '0' : (ch & 0x5F) - 'A' + 10; };
            text[tl++] = (char)((hv(q[0]) << 4) | hv(q[1])); q += 2;
        }
        text[tl] = 0;
        const char* sp = strstr(text, "HTTP/");
        if (sp && (sp = strchr(sp, ' '))) status = atoi(sp + 1);
        else status = -1;
    }

    snprintf(c, sizeof c, "AT+CHTTPDISCON=%d", id);  cmd(c, "OK", 3000);
    snprintf(c, sizeof c, "AT+CHTTPDESTROY=%d", id); cmd(c, "OK", 3000);
    return status;
}
