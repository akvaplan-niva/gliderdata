#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>
#include <stdlib.h>   // strtol, strtod

#include "glider.h"
#include "sensor.h"

// ===== Debug via RTT (never touches UARTs) =====
#define DEBUG_RTT 1
#if DEBUG_RTT
#include <zephyr/sys/printk.h>
#endif

#define UART0_NODE DT_NODELABEL(uart0) // Glider
#define UART1_NODE DT_NODELABEL(uart1) // Sensor
#if !DT_NODE_EXISTS(UART0_NODE) || !DT_NODE_EXISTS(UART1_NODE)
#error "uart0/uart1 not found in device tree"
#endif

static const struct device *const uart0_dev = DEVICE_DT_GET(UART0_NODE); // glider
static const struct device *const uart1_dev = DEVICE_DT_GET(UART1_NODE); // sensor

// ------ Minimal token matcher ------
typedef struct {
    const char *pat;
    size_t      len;
    size_t      pos;
    bool        hit;
} stream_match_t;

static inline void matcher_init(stream_match_t *m, const char *pat){
    m->pat = pat; m->len = strlen(pat); m->pos = 0; m->hit = false;
}
static inline bool matcher_feed(stream_match_t *m, uint8_t ch){
    if (m->hit || m->len == 0) return m->hit;
    if (ch == (uint8_t)m->pat[m->pos]) {
        if (++m->pos == m->len) m->hit = true;
    } else {
        m->pos = (ch == (uint8_t)m->pat[0]) ? 1 : 0;
    }
    return m->hit;
}

// ------ Line accumulator ------
typedef struct { char buf[512]; size_t len; int64_t last_ms; } line_acc_t;
static inline void line_acc_reset(line_acc_t *a){ a->len = 0; a->buf[0] = '\0'; a->last_ms = k_uptime_get(); }
static inline bool line_acc_add(line_acc_t *a, uint8_t ch){
    if (a->len < sizeof(a->buf)-1) { a->buf[a->len++] = (char)ch; a->buf[a->len] = '\0'; }
    a->last_ms = k_uptime_get();
    return (ch=='\n') || (ch=='\r');
}
static inline bool line_acc_stalled(const line_acc_t *a){
    return (a->len > 0 && (a->len >= sizeof(a->buf)-4 || (k_uptime_get() - a->last_ms) > 150));
}

// --------- Helpers: SW on UART0 (with XOR checksum) ----------
static uint8_t xor_checksum(const char *payload){ uint8_t cs=0; for (const char *p=payload; *p; ++p) cs^=(uint8_t)(*p); return cs; }
static inline char hex_hi(uint8_t v){ v>>=4; return (v<10)?('0'+v):('A'+(v-10)); }
static inline char hex_lo(uint8_t v){ v&=0x0F; return (v<10)?('0'+v):('A'+(v-10)); }

static inline void uart0_send_byte(uint8_t b){ uart_poll_out(uart0_dev, b); }
static inline void uart1_send_bytes(const uint8_t *d, size_t n){ for (size_t i=0;i<n;i++) uart_poll_out(uart1_dev, d[i]); }

// Integer-only formatter for depth with two decimals (no %f needed)
static void send_sw_on_uart0_depth2(uint8_t index, double value){
    long long centi = llround(value * 100.0);
    bool neg = (centi < 0);
    if (neg) centi = -centi;
    unsigned long long whole = (unsigned long long)(centi / 100);
    unsigned long long frac  = (unsigned long long)(centi % 100);

    char payload[48];
    int n = snprintk(payload, sizeof(payload), "SW,%u:%s%llu.%02llu",
                     (unsigned)index, neg?"-":"", whole, frac);
    if (n <= 0 || n >= (int)sizeof(payload)) return;
    uint8_t cs = xor_checksum(payload);
    uart0_send_byte('$');
    for (int i=0;i<n;i++) uart0_send_byte((uint8_t)payload[i]);
    uart0_send_byte('*'); uart0_send_byte((uint8_t)hex_hi(cs)); uart0_send_byte((uint8_t)hex_lo(cs));
    uart0_send_byte('\r'); uart0_send_byte('\n');
#if DEBUG_RTT
    printk("[RTT] ->U0 SW depth idx%u: $%s*%02X\\r\\n\n", index, payload, cs);
#endif
}
static void send_sw_on_uart0_uint(uint8_t index, uint32_t value){
    char payload[32];
    int n = snprintk(payload, sizeof(payload), "SW,%u:%u", (unsigned)index, (unsigned)value);
    if (n <= 0 || n >= (int)sizeof(payload)) return;
    uint8_t cs = xor_checksum(payload);
    uart0_send_byte('$');
    for (int i=0;i<n;i++) uart0_send_byte((uint8_t)payload[i]);
    uart0_send_byte('*'); uart0_send_byte((uint8_t)hex_hi(cs)); uart0_send_byte((uint8_t)hex_lo(cs));
    uart0_send_byte('\r'); uart0_send_byte('\n');
#if DEBUG_RTT
    printk("[RTT] ->U0 SW uint idx%u: $%s*%02X\\r\\n\n", index, payload, cs);
#endif
}

// --------- Sensor cmds on UART1 ----------
static inline void sensor_send_cstr_noeol(const char *s){
    uart1_send_bytes((const uint8_t*)s, strlen(s));
}
static inline void sensor_send_lf(void){
    static const char e[] = "\n";
    uart1_send_bytes((const uint8_t*)e, 1);
}
static inline void sensor_send_stop(void){
    static const char stop_cmd[] = "$stop;";
    sensor_send_cstr_noeol(stop_cmd);
    sensor_send_lf(); // LF-terminated
#if DEBUG_RTT
    printk("[RTT] ->U1 STOP: %s\\n\n", stop_cmd);
#endif
}

// ==== Minimal UNIX epoch -> UTC calendar (no timezone/DST) ====
static void epoch_to_ymdhms(uint32_t t, int *outY, int *outM, int *outD, int *outh, int *outm, int *outs)
{
    uint32_t days = t / 86400U;
    uint32_t rem  = t % 86400U;
    int hh = (int)(rem / 3600U); rem %= 3600U;
    int mm = (int)(rem / 60U);
    int ss = (int)(rem % 60U);

    int64_t z = (int64_t)days + 719468;
    int64_t era = (z >= 0 ? z : z - 146096) / 146097;
    unsigned doe = (unsigned)(z - era*146097);
    unsigned yoe = (doe - doe/1460 + doe/36524 - doe/146096) / 365;
    int y = (int)yoe + (int)(era)*400;
    unsigned doy = doe - (365*yoe + yoe/4 - yoe/100);
    unsigned mp = (5*doy + 2)/153;
    unsigned mon_u = mp + (mp < 10 ? 3 : -9);
    unsigned day_u = doy - (153*mp+2)/5 + 1;
    y += (mon_u <= 2);

    *outY = y; *outM = (int)mon_u; *outD = (int)day_u;
    *outh = hh; *outm = mm; *outs = ss;
}

// Build and send the start command using the provided UTC date/time
static void sensor_send_start_with_epoch(uint32_t epoch_sec)
{
    int Y,M,D,h,m,s;
    epoch_to_ymdhms(epoch_sec, &Y,&M,&D,&h,&m,&s);

    char body[64];
    // $start:ACQ_CSCS_002H,YYYYMMDD,HHMMSS;\n
    int n = snprintk(body, sizeof(body), "$start:ACQ_CSCS_002H,%04d%02d%02d,%02d%02d%02d;",
                     Y,M,D,h,m,s);
    if (n <= 0) return;

    sensor_send_cstr_noeol(body);
    sensor_send_lf();
#if DEBUG_RTT
    printk("[RTT] ->U1 START: %s\\n\n", body);
#endif
}

// ------ Non-blocking poll helper ------
static inline void poll_drain_uart(const struct device *dev, void (*on_byte)(uint8_t)){
    unsigned char ch;
    while (uart_poll_in(dev, &ch) == 0) {
        on_byte((uint8_t)ch);
    }
}

// ===== Global state =====
static stream_match_t m_glider, m_sensor;
static line_acc_t acc0, acc1;

static bool glider_seen = false;
static bool sensor_seen = false;
static bool glider_sd_seen = false;

static bool sensor_started = false;
static int  last_mode = -1;

static double last_epoch_f = 0.0;     // from SD,3
static bool   have_epoch   = false;

static uint32_t lpm_count = 0;        // count of LPM_DATA lines since last start
static double   lpm_last_depth = 0.0; // depth from the last LPM_DATA before stop

static volatile bool stop_in_progress = false; // avoid re-entrant stops

// Passthrough mode state
static bool in_passthrough_mode = false;

// --- Parse helpers ---
static bool starts_with(const char *s, const char *prefix){
    return strncmp(s, prefix, strlen(prefix)) == 0;
}

// Check if line matches a command (with CR/LF variations)
static bool line_matches_cmd(const char *line, const char *cmd){
    size_t cmd_len = strlen(cmd);
    if (strncmp(line, cmd, cmd_len) != 0) return false;
    // After command, should see \r or \n or \r\n or end
    char ch = line[cmd_len];
    return (ch == '\0' || ch == '\r' || ch == '\n');
}

// parse "$SD,..." → extract epoch (field 3), depth (field 4), mode (field 5)
static void parse_sd_fields(const char *line, double *epoch_out, double *depth_out, int *mode_out)
{
    const char *p = strstr(line, "$SD,");
    if (!p) return;
    char tmp[256];
    strncpy(tmp, p+4, sizeof(tmp)-1); // skip "$SD,"
    tmp[sizeof(tmp)-1] = '\0';
    char *star = strchr(tmp, '*');
    if (star) *star = '\0';

    char *tok, *ctx = NULL;
    for (tok = strtok_r(tmp, ",", &ctx); tok; tok = strtok_r(NULL, ",", &ctx)) {
        int idx = -1;
        const char *colon = strchr(tok, ':');
        if (!colon) continue;
        idx = (int)strtol(tok, NULL, 10);
        const char *val = colon + 1;

        if (idx == 3 && epoch_out) {
            double e = strtod(val, NULL);
            *epoch_out = e;
        } else if (idx == 4 && depth_out) {
            *depth_out = strtod(val, NULL);
        } else if (idx == 5 && mode_out) {
            *mode_out = (int)strtol(val, NULL, 10);
        }
    }
}

// parse "LPM_DATA,<depth>,..." → return true and set *depth_out if parsed
static bool parse_lpm_depth(const char *line, double *depth_out)
{
    if (!starts_with(line, "LPM_DATA,")) return false;
    const char *p = line + strlen("LPM_DATA,");
    char *endp = NULL;
    double d = strtod(p, &endp);
    if (endp == p) return false; // no number
    *depth_out = d;
    return true;
}

// ------ Byte handlers for phase 1 (initial window) ------
static void on_u0_phase1(uint8_t b){
    if (!m_glider.hit && matcher_feed(&m_glider, b)) glider_seen = true;
    bool eol = line_acc_add(&acc0, b);
    if (eol || line_acc_stalled(&acc0)) {
        if (strstr(acc0.buf, "$SD,")) {
            glider_sd_seen = true;
            double ep=0.0, dep=0.0; int md=-1;
            parse_sd_fields(acc0.buf, &ep, &dep, &md);
            if (ep > 0.0) { last_epoch_f = ep; have_epoch = true; }
#if DEBUG_RTT
            printk("[RTT] U0 SD candidate: \"%s\" (epoch=%.3f mode=%d)\n", acc0.buf, last_epoch_f, md);
#endif
        }
        line_acc_reset(&acc0);
    }
}
static void on_u1_phase1(uint8_t b){
    if (!m_sensor.hit && matcher_feed(&m_sensor, b)) sensor_seen = true;
    bool eol = line_acc_add(&acc1, b);
    if (eol || line_acc_stalled(&acc1)) {
#if DEBUG_RTT
        if (strstr(acc1.buf, "$startack;")) printk("[RTT] U1 startack seen (phase1): \"%s\"\n", acc1.buf);
#endif
        line_acc_reset(&acc1);
    }
}

// ------ Special handler while waiting for stopack ------
static void on_u0_during_stop(uint8_t b){
    // Keep epoch/mode fresh but never start another stop
    bool eol = line_acc_add(&acc0, b);
    if (eol || line_acc_stalled(&acc0)) {
        if (strstr(acc0.buf, "$SD,")) {
            double ep=0.0, dep=0.0; int md=-1;
            parse_sd_fields(acc0.buf, &ep, &dep, &md);
            if (ep > 0.0) { last_epoch_f = ep; have_epoch = true; }
            if (md >= 0) last_mode = md;
#if DEBUG_RTT
            printk("[RTT] U0 (during stop) SD epoch=%.3f mode=%d\n", last_epoch_f, last_mode);
#endif
        }
        line_acc_reset(&acc0);
    }
}

// Forward declarations
static void run_stop_sequence(bool report_depth_if_dive, bool restart_after);
static void run_passthrough_mode(void);

// ------ Byte handlers for run phase ------
static void on_u0_run(uint8_t b){
    if (!glider_seen && matcher_feed(&m_glider, b)) glider_seen = true;

    bool eol = line_acc_add(&acc0, b);
    if (eol || line_acc_stalled(&acc0)) {
#if DEBUG_RTT
        // Print first 20 chars of line safely
        printk("U0: ");
        for (size_t i = 0; i < acc0.len && i < 20; i++) {
            if (acc0.buf[i] >= 32 && acc0.buf[i] < 127) {
                printk("%c", acc0.buf[i]);
            } else {
                printk(".");
            }
        }
        printk("\n");
#endif
        // Check for $MIRROR command to enter passthrough mode
        if (line_matches_cmd(acc0.buf, "$MIRROR")) {
#if DEBUG_RTT
            printk("RTT: MIRROR matched\n");
#endif
            in_passthrough_mode = true;
            line_acc_reset(&acc0);
            run_passthrough_mode();
            // After exiting passthrough, reset accumulators
            line_acc_reset(&acc0);
            line_acc_reset(&acc1);
            return;
        }

        // BY (end) handling: stop sensor and DO NOT restart
        if (strstr(acc0.buf, "$BY")) {
#if DEBUG_RTT
            printk("[RTT] U0 BY seen -> STOP (no restart)\n");
#endif
            if (!stop_in_progress) {
                stop_in_progress = true;
                run_stop_sequence(true /*depth ok*/, false /*no restart*/);
                stop_in_progress = false;
            }
            line_acc_reset(&acc0);
            return;
        }

        if (strstr(acc0.buf, "$SD,")) {
            double ep=0.0, dep=0.0; int md=-1;
            parse_sd_fields(acc0.buf, &ep, &dep, &md);
            if (ep > 0.0) { last_epoch_f = ep; have_epoch = true; }
#if DEBUG_RTT
            printk("[RTT] U0 SD: \"%s\" (epoch=%.3f, mode=%d)\n", acc0.buf, last_epoch_f, md);
#endif
            if (!stop_in_progress && md >= 0) {
                if (last_mode < 0) {
                    last_mode = md;
                } else if (md != last_mode && sensor_started) {
                    // Mode change → stop sequence
                    bool prev_was_dive = (last_mode == 1) && (md != 1);
#if DEBUG_RTT
                    printk("[RTT] Mode change %d->%d: STOP (retries), report SW, restart\n", last_mode, md);
#endif
                    stop_in_progress = true;
                    run_stop_sequence(prev_was_dive /*depth only if dive leaving*/, true /*restart*/);
                    stop_in_progress = false;
                    last_mode = md;
                    line_acc_reset(&acc0);
                    return;
                } else {
                    last_mode = md;
                }
            } else if (md >= 0) {
                last_mode = md;
            }
        }
        line_acc_reset(&acc0);
    }
}

static void on_u1_run(uint8_t b){
    if (!sensor_seen && matcher_feed(&m_sensor, b)) sensor_seen = true;
    bool eol = line_acc_add(&acc1, b);
    if (eol || line_acc_stalled(&acc1)) {
        if (strstr(acc1.buf, "$startack;")) {
#if DEBUG_RTT
            printk("[RTT] U1 startack received\n");
#endif
        } else {
            double d;
            if (parse_lpm_depth(acc1.buf, &d)) {
                lpm_count++;
                lpm_last_depth = d;
#if DEBUG_RTT
                printk("[RTT] U1 LPM_DATA #%u depth=%.2f\n", lpm_count, lpm_last_depth);
#endif
            }
        }
        line_acc_reset(&acc1);
    }
}

static void run_stop_sequence(bool report_depth_if_dive, bool restart_after)
{
    bool stopacked = false;

    for (int attempt=1; attempt<=3 && !stopacked; ++attempt) {
        sensor_send_stop(); // LF-terminated
        int64_t deadline = k_uptime_get() + 1000; // exactly 1 s wait per your spec

        // Poll during the 1 s window
        do {
            // Glider: keep epoch/mode fresh, but don't re-enter stop
            poll_drain_uart(uart0_dev, on_u0_during_stop);

            // Sensor: look for stopack and continue counting LPMs until ack
            unsigned char ch;
            while (uart_poll_in(uart1_dev, &ch) == 0) {
                bool eol1 = line_acc_add(&acc1, ch);
                if (eol1 || line_acc_stalled(&acc1)) {
                    if (strstr(acc1.buf, "$stopack;")) {
#if DEBUG_RTT
                        printk("[RTT] U1 stopack received\n");
#endif
                        stopacked = true;
                    } else {
                        double d;
                        if (parse_lpm_depth(acc1.buf, &d)) {
                            lpm_count++;
                            lpm_last_depth = d;
#if DEBUG_RTT
                            printk("[RTT] U1 LPM_DATA #%u depth=%.2f (during stop wait)\n", lpm_count, lpm_last_depth);
#endif
                        }
                    }
                    line_acc_reset(&acc1);
                }
            }
            k_sleep(K_MSEC(5));
        } while (!stopacked && k_uptime_get() < deadline);

        if (!stopacked) {
#if DEBUG_RTT
            printk("[RTT] stopack not seen (attempt %d)\n", attempt);
#endif
        }
    }

    // Report metrics on UART0:
    // - always send count on idx=1
    // - send depth on idx=2 only if we stopped while leaving a DIVE (1→2, etc.)
    send_sw_on_uart0_uint(1, lpm_count);
    if (report_depth_if_dive) {
        send_sw_on_uart0_depth2(2, lpm_last_depth);
    }

    // Prepare for next leg
    lpm_count = 0;
    lpm_last_depth = 0.0;

    // Restart if requested and we have epoch
    if (restart_after && have_epoch) {
        uint32_t epoch_sec = (uint32_t)floor(last_epoch_f);
        sensor_send_start_with_epoch(epoch_sec);
        sensor_started = true;
    } else if (restart_after && !have_epoch) {
#if DEBUG_RTT
        printk("[RTT] No epoch yet; deferring restart until SD,3 arrives\n");
#endif
        sensor_started = false;
    }
}

// ===== UART Passthrough Mode =====
// Enter passthrough: forward all bytes bidirectionally until $QUIT\r\n is seen
static void run_passthrough_mode(void)
{
#if DEBUG_RTT
    printk("RTT: Entering passthrough\n");
#endif
    // Use static buffers instead of stack allocation to avoid overflow
    static line_acc_t cmd_acc_u0, cmd_acc_u1;
    line_acc_reset(&cmd_acc_u0);
    line_acc_reset(&cmd_acc_u1);

    while (in_passthrough_mode) {
        // Forward from UART0 (glider) to UART1 (sensor)
        unsigned char ch0;
        while (uart_poll_in(uart0_dev, &ch0) == 0) {
            // Forward byte to sensor first
            uart_poll_out(uart1_dev, ch0);
            
            // Check for $QUIT command
            bool eol = line_acc_add(&cmd_acc_u0, ch0);
            if (eol || line_acc_stalled(&cmd_acc_u0)) {
                if (line_matches_cmd(cmd_acc_u0.buf, "$QUIT")) {
#if DEBUG_RTT
                    printk("RTT: QUIT on U0\n");
#endif
                    in_passthrough_mode = false;
                    line_acc_reset(&cmd_acc_u0);
                    return;
                }
                line_acc_reset(&cmd_acc_u0);
            }
        }

        // Forward from UART1 (sensor) to UART0 (glider)
        unsigned char ch1;
        while (uart_poll_in(uart1_dev, &ch1) == 0) {
            // Forward byte to glider first
            uart_poll_out(uart0_dev, ch1);
            
            // Check for $QUIT command
            bool eol = line_acc_add(&cmd_acc_u1, ch1);
            if (eol || line_acc_stalled(&cmd_acc_u1)) {
                if (line_matches_cmd(cmd_acc_u1.buf, "$QUIT")) {
#if DEBUG_RTT
                    printk("RTT: QUIT on U1\n");
#endif
                    in_passthrough_mode = false;
                    line_acc_reset(&cmd_acc_u1);
                    return;
                }
                line_acc_reset(&cmd_acc_u1);
            }
        }

        k_sleep(K_USEC(100)); // Small delay to avoid busy-waiting
    }
}

void main(void){
#if DEBUG_RTT
    printk("RTT: Main start\n");
#endif
    if (!device_is_ready(uart0_dev) || !device_is_ready(uart1_dev)) {
#if DEBUG_RTT
        printk("RTT: ERROR UARTs not ready\n");
#endif
        while (1) { k_msleep(1000); }
    }

    // Send startup messages on each UART to verify they're working
    // UART0: Send byte by byte
    uart_poll_out(uart0_dev, 'G');
    uart_poll_out(uart0_dev, 'L');
    uart_poll_out(uart0_dev, 'I');
    uart_poll_out(uart0_dev, 'D');
    uart_poll_out(uart0_dev, 'E');
    uart_poll_out(uart0_dev, 'R');
    uart_poll_out(uart0_dev, '\r');
    uart_poll_out(uart0_dev, '\n');
    
    k_msleep(100);
    
    // UART1: Send byte by byte
    uart_poll_out(uart1_dev, 'S');
    uart_poll_out(uart1_dev, 'E');
    uart_poll_out(uart1_dev, 'N');
    uart_poll_out(uart1_dev, 'S');
    uart_poll_out(uart1_dev, 'O');
    uart_poll_out(uart1_dev, 'R');
    uart_poll_out(uart1_dev, '\r');
    uart_poll_out(uart1_dev, '\n');

    // Init state
    matcher_init(&m_glider, GLIDER_EXPECTED_STR);
    matcher_init(&m_sensor,  SENSOR_EXPECTED_STR);
    line_acc_reset(&acc0);
    line_acc_reset(&acc1);
    glider_seen = sensor_seen = glider_sd_seen = false;
    sensor_started = false;
    last_mode = -1;
    have_epoch = false; last_epoch_f = 0.0;
    lpm_count = 0; lpm_last_depth = 0.0;
    stop_in_progress = false;
    in_passthrough_mode = false;

#if DEBUG_RTT
    printk("[RTT] Ready. Waiting for glider/sensor activity (SD,3 epoch; SD,5 mode)\n");
    printk("[RTT] Commands: $MIRROR (passthrough), $QUIT (exit passthrough)\n");
#endif

    // No initial SW report - board is always powered by glider

    // Brief initial detection phase to see if sensor responds
    int64_t init_deadline = k_uptime_get() + SENSOR_TIMEOUT_MS;
    while (k_uptime_get() < init_deadline && !sensor_seen) {
        poll_drain_uart(uart0_dev, on_u0_phase1);
        poll_drain_uart(uart1_dev, on_u1_phase1);
        k_sleep(K_MSEC(1));
    }

#if DEBUG_RTT
    if (sensor_seen) {
        printk("[RTT] Sensor detected: '%s'\n", SENSOR_EXPECTED_STR);
    } else {
        printk("[RTT] Sensor not detected within timeout\n");
    }
#endif

    // Run phase
    for (;;) {
        // Drain both UARTs
        poll_drain_uart(uart0_dev, on_u0_run);
        poll_drain_uart(uart1_dev, on_u1_run);

        // Start condition (requires epoch so we can format date/time)
        if (!sensor_started && sensor_seen && (glider_seen || glider_sd_seen) && have_epoch) {
            uint32_t epoch_sec = (uint32_t)floor(last_epoch_f);
            sensor_send_start_with_epoch(epoch_sec);
            sensor_started = true;
        }

        k_sleep(K_MSEC(2));
    }
}

