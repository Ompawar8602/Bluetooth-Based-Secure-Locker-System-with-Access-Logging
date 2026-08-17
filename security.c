/*=============================================================================
 * File        : security.c
 * Project     : Bluetooth-Based Secure Locker with Access Logging
 * Description : Ties together tamper detection, RTC-timestamped access
 *               logging (sent out over UART0 for a PC to capture), and
 *               one-time initialisation of the default EEPROM passwords.
 *
 * Wiring:
 *   Tamper switch -> P0.4, active LOW (switch pulls the pin to GND when
 *                    the enclosure is opened/tampered with; an external
 *                    pull-up resistor keeps the pin HIGH when closed)
 *===========================================================================*/
#include <lpc214x.h>
#include <string.h>
#include "security.h"
#include "defines.h"
#include "lcd.h"
#include "uart.h"
#include "rtc.h"
#include "eeprom.h"
#include "buzzer.h"

#define TAMPER_PIN   (1UL << 4)   /* P0.4 - tamper switch input */

/* Configure the tamper switch pin as a GPIO input. */
void security_init(void)
{
    PINSEL0 &= ~(3UL << 8);   /* P0.4 as plain GPIO (not an alternate function) */
    IO0DIR  &= ~TAMPER_PIN;   /* Configure as input */
}

/* Read the current state of the tamper switch.
 * The switch is active LOW, so a HIGH pin reading means "not tampered"
 * (returns 0) and a LOW reading means "tampered" (returns 1). */
u8 tamper_detected(void)
{
    if (IO0PIN & TAMPER_PIN) return 0;   /* Pin HIGH -> switch closed -> OK       */
    else                     return 1;   /* Pin LOW  -> switch open  -> tampered  */
}

/* Print just the "[DD/MM/YYYY HH:MM:SS] " timestamp bracket, with no
 * trailing newline. Used by multi-part UART lines (e.g. "[BT] Received
 * password: 1234") that build up more than one piece on the same line,
 * so every line on the terminal - not just single-message ones - carries
 * the same timestamp format. */
void log_prefix(void)
{
    char stamp[20];
    rtc_get_stamp(stamp);

    uart0_string("[");
    uart0_string(stamp);
    uart0_string("] ");
}

/* Print a single timestamped log line over UART0 in the form:
 * "[DD/MM/YYYY HH:MM:SS] <message>\r\n"
 * This is intended to be captured by a PC terminal for an audit trail. */
void log_event(const char *msg)
{
    log_prefix();
    uart0_string(msg);
    uart0_string("\r\n");
}

/* Poll the tamper switch once and, on a fresh LOW->tampered transition
 * (edge-detected via 'old_state' so the alert only fires once per event
 * rather than continuously while the switch stays open), show an alert
 * on the LCD, log the event, and sound the buzzer. */
void check_tamper_and_alert(void)
{
    static u8 old_state = 0;

    if (tamper_detected())
    {
        if (old_state == 0)   /* Only trigger on the transition into "tampered" */
        {
            old_state = 1;
            lcd_clear();
            lcd_string("TAMPER ALERT");
            log_event("Tamper detected");
            buzzer_alert(5);
        }
    }
    else
    {
        old_state = 0;   /* Switch closed again -> re-arm for the next event */
    }
}

/* On first boot (or after a blank/corrupted EEPROM), write the 4-byte
 * "LKR1" magic marker plus the two factory-default passwords
 * (Level-1/Bluetooth = "1234", Level-2/keypad = "5678") into the EEPROM.
 * On subsequent boots the marker will already match and this function
 * does nothing, preserving any passwords the admin has since changed. */
void ensure_default_passwords(void)
{
    char magic[5];

    eeprom_read_str(EEPROM_MAGIC_ADDR, magic, 4);

    if (strcmp(magic, "LKR1") != 0)
    {
        eeprom_write_str(EEPROM_MAGIC_ADDR, "LKR1", 4);
        eeprom_write_str(EEPROM_L1_ADDR, "1234", PWD_LEN);
        eeprom_write_str(EEPROM_L2_ADDR, "5678", PWD_LEN);
        log_event("EEPROM initialized with default passwords");
    }
}
