<div align="center">

# 🔐 SecureLocker
### Bluetooth-Based Two-Factor Secure Locker with Access Logging

*A dual-authentication embedded access-control system built on the ARM7 (LPC2148), combining Bluetooth and physical keypad verification with real-time tamper detection and audit logging.*

![Platform](https://img.shields.io/badge/Platform-ARM7%20LPC2148-blue?style=flat-square)
![Language](https://img.shields.io/badge/Language-Embedded%20C-00599C?style=flat-square&logo=c&logoColor=white)
![IDE](https://img.shields.io/badge/IDE-Keil%20µVision-orange?style=flat-square)
![Bluetooth](https://img.shields.io/badge/Wireless-HC--05%20Bluetooth-0082FC?style=flat-square&logo=bluetooth&logoColor=white)
![Status](https://img.shields.io/badge/Status-Complete-brightgreen?style=flat-square)
![License](https://img.shields.io/badge/License-MIT-lightgrey?style=flat-square)

</div>

---

## 📖 Overview

**SecureLocker** is a two-factor embedded access-control system built on the **NXP LPC2148 (ARM7TDMI-S)**. A user first sends a 4-digit password from a phone over **Bluetooth** (Level-1); if correct, they're prompted to enter a second 4-digit password on a **physical keypad** (Level-2). Only when both match does the system drive a **DC motor** to open — and then automatically close — the locker. Every significant event is timestamped via the on-chip RTC and streamed out over UART as an audit log.

---

## ✨ Highlights

| | Feature | Description |
|---|---|---|
| 📱 | **Bluetooth Authentication** | Level-1 password entered via HC-05 from any Bluetooth serial app |
| 🔢 | **Keypad Authentication** | Level-2 password entered on a 4×4 matrix keypad after Level-1 succeeds |
| ⚙️ | **Motor-Driven Locking** | L293D H-bridge drives a DC motor to physically open/close the locker |
| 🛡️ | **Tamper Detection** | Interrupt-monitored switch triggers an instant alert if the enclosure is opened |
| 🕒 | **RTC Audit Logging** | Every event is timestamped and streamed over UART for a PC-side access log |
| 💾 | **EEPROM Password Storage** | Passwords persist across power cycles via I2C (AT24C256) |
| 🧑‍💻 | **Admin Configuration Menu** | Change passwords, set the clock, and configure an alarm — all on-device |
| ⏰ | **Alarm System** | Configurable time-based alarm with buzzer alert |
| 🔊 | **Audible Feedback** | Buzzer confirms tamper events, denied access, and alarm triggers |
| 🖥️ | **LCD Status Display** | 16×2 LCD shows live system state — standby, prompts, results, alerts |

---

## 🔌 Hardware & Pin Connections

| Module | Signal | LPC2148 Pin |
|---|---|---|
| 🖥️ **LCD (16×2, 4-bit mode)** | RS (Register Select) | P0.16 |
| | EN (Enable / Strobe) | P0.17 |
| | D4 | P0.18 |
| | D5 | P0.19 |
| | D6 | P0.20 |
| | D7 | P0.21 |
| | R/W | Tied to GND (write-only, not MCU-driven) |
| 🖧 **UART0** (debug/log → PC) | TXD0 | P0.0 |
| | RXD0 | P0.1 |
| 💾 **I2C0** (AT24C256 EEPROM) | SCL0 | P0.2 |
| | SDA0 | P0.3 |
| 🛡️ **Tamper Switch** | Signal (active LOW) | P0.4 |
| 🧑‍💻 **Admin Button** | EINT2 (falling edge) | P0.7 |
| 📶 **UART1** (HC-05 Bluetooth) | TXD1 | P0.8 |
| | RXD1 | P0.9 |
| 🔢 **Keypad (4×4 matrix)** | Row 1–4 (outputs) | P1.16 – P1.19 |
| | Col 1–4 (inputs) | P1.20 – P1.23 |
| ⚙️ **Motor Driver (L293D)** | IN1 | P1.24 |
| | IN2 | P1.25 |
| 🔊 **Buzzer** | Signal | P1.26 |

**Core clock:** 60 MHz (12 MHz crystal, PLL M=5 / P=2) · **PCLK:** 15 MHz
**I2C slave address:** AT24C256 EEPROM responds at `0xA0` (all address pins tied low)

---

## 🔑 Default Credentials

| Level | Method | Factory Default |
|---|---|---|
| Level 1 | Bluetooth | `1234` |
| Level 2 | Keypad | `5678` |

> Written to EEPROM only on first boot (`"LKR1"` magic marker). Change both via the on-device admin menu.

---

## 🧑‍💻 Admin Menu

Triggered by an external push-button (EINT2):

```
1 = CLK Setting     → Time / Date / Day
2 = Alarm           → Set time / Toggle on-off / Reset
3 = Password        → Change Level-1 or Level-2 password
4 = Set              → Save & exit
D = Back / Cancel
```

Auto-exits after **15 seconds** of inactivity (ATM-style timeout).

---

## 🗂️ Project Structure

```
SecureLocker/
├── projectmain.c     # System init, main loop, two-factor auth flow
├── bluetooth.c/.h     # UART1 + HC-05 command handling
├── keypad.c/.h        # 4x4 matrix keypad scanning
├── lcd.c/.h            # 16x2 LCD driver (4-bit mode)
├── security.c/.h      # Tamper detection, event logging, EEPROM defaults
├── menu.c/.h           # Admin menu, CLK/Alarm/Password sub-menus
├── eeprom.c/.h         # I2C driver for AT24C256 password storage
├── rtc.c/.h             # On-chip RTC read/write
├── motor.c/.h           # L293D motor control (open/close sequence)
├── buzzer.c/.h          # Audible alert driver
├── uart.c/.h             # UART0 debug/log output
└── Startup.s              # ARM7 startup / vector table
```

---

## 🔍 How It Works — Full System Walkthrough

This section walks through everything that happens, from the moment the device is powered on to the locker closing again — in plain language, no code required to follow along.

<img src="" alt="Photo" width="500">

### 1️⃣ Power On & Initialization
The moment power is applied, the LPC2148 configures its clock (PLL), then initializes every peripheral it needs: the LCD, UART (both the debug port and the Bluetooth port), the I2C bus (for EEPROM), the RTC, the keypad, the motor driver, and the buzzer. If this is the very first boot ever, the system also writes the factory-default passwords (`1234` for Bluetooth, `5678` for keypad) into EEPROM so they survive future power cycles.

### 2️⃣ Welcome Screen & Standby
Once setup is complete, the 16×2 LCD displays a welcome message and the system enters **standby**, quietly waiting for one of three things to happen: a Bluetooth password arriving, the admin button being pressed, or a tamper/alarm condition triggering.

### 3️⃣ Level-1 Authentication — Bluetooth
The user opens any Bluetooth serial app on their phone, connects to the HC-05 module, and types their 4-digit password followed by `#` (e.g. `1234#`). The system reads this over UART1 and compares it against the Level-1 password stored in EEPROM.
- ❌ **Wrong password** → LCD shows "ACCESS DENIED," the buzzer sounds a short alert, the attempt is logged with a timestamp, and the system returns to standby (or triggers lockout mode after repeated failures).
- ✅ **Correct password** → LCD shows "LEVEL1 OK," and the system immediately asks for the second password.

### 4️⃣ Level-2 Authentication — Keypad
The user now types their second 4-digit password directly on the physical 4×4 keypad (each digit shown on the LCD as `*` for privacy). This is checked against the Level-2 password in EEPROM.
- ❌ **Wrong password** → Same denial flow as Level-1: alert, log entry, return to standby.
- ✅ **Correct password** → Access is granted.

### 5️⃣ Locker Unlocks
With both factors verified, the LCD displays "ACCESS GRANTED" and the microcontroller drives the DC motor (through the L293D H-bridge) to physically unlock and open the locker.

### 6️⃣ Auto-Lock
After holding open for a short, fixed delay, the motor automatically reverses to close and re-lock the locker — no manual step required from the user.

### 7️⃣ Logging Every Event
Every meaningful event along the way — boot, password attempts (success or failure), locker open/close, tamper detection, admin logins, and password changes — is timestamped using the on-chip RTC and streamed out over UART0. If a laptop is connected via a USB-to-TTL converter, all of this appears as a live, readable access log in real time.

### 8️⃣ Tamper Detection (runs continuously)
In parallel with everything above, the system continuously watches a tamper switch wired to the enclosure. If the locker is physically opened or the switch is triggered without a valid unlock sequence, the system immediately shows a tamper alert on the LCD, sounds the buzzer, and logs the event — regardless of what else was happening at the time.

### 9️⃣ Admin Mode (on demand)
Pressing the dedicated admin button interrupts the normal flow and opens an on-device menu:
- **Set the clock** (time/date/day)
- **Configure the alarm** (set a trigger time, turn it on/off, or reset it)
- **Change passwords** (Level-1 or Level-2)
- **Save & exit** back to standby

The menu automatically times out and exits after 15 seconds of inactivity, so it can't be left open accidentally.

### 🔟 Alarm (optional, if configured)
If an alarm time has been set via the admin menu, the system checks the RTC on every loop pass. When the current time matches the alarm time, it sounds the buzzer and displays an alert — which can only be stopped by entering the admin menu and disabling or resetting the alarm.

---
##  Software Architecture 

<img src="" alt="Photo" width="500">

---

## 🔌 Hardware Block Diagram

<img src="" alt="Photo" width="500">

---

## 🔄 System Workflow

<img src="" alt="Photo" width="500">

---

## 🌿 Branching Strategy

This repo follows a simple feature-branch workflow, keeping `main` always in a stable, buildable state:

```
main                        ← stable, tested releases only
 │
 ├── dev                    ← integration branch for ongoing work
 │    │
 │    ├── feature/bluetooth-uart-fix     (PLL init + UART RX ring buffer fix)
 │    ├── feature/tamper-detection       (EINT-based tamper switch + alerts)
 │    ├── feature/admin-menu             (CLK / Alarm / Password sub-menus)
 │    └── feature/alarm-system           (RTC-based alarm merge)
 │
 └── hotfix/*                ← urgent fixes branched directly from main
```

**Workflow:**
1. New work branches off `dev` as `feature/<name>`
2. Once tested in Keil (build + hardware check), merge into `dev`
3. `dev` is merged into `main` only after a full end-to-end hardware test of both auth levels, tamper detection, and the admin menu
4. Critical bugs found on `main` get a short-lived `hotfix/<name>` branch merged straight back into `main` (and back into `dev`)

> Currently maintained as a single `main` branch for simplicity; adopt the structure above if collaborating or tracking feature history going forward.

---

## 🛠️ Build & Flash

1. Open the `.uvproj` in **Keil µVision** (ARM/MDK toolchain)
2. Build — produces `.axf` and `.hex`
3. Flash the `.hex` to the LPC2148 via ISP/JTAG
4. Wire peripherals per the pin table above

---

## 🚀 Tech Stack

`Embedded C` · `ARM7TDMI-S (LPC2148)` · `Keil µVision` · `UART` · `I2C` · `GPIO Interrupts` · `RTC`

---

<div align="center">

Built by **Om Pawar** · Electronics & Telecommunication Engineering
[LinkedIn](https://www.linkedin.com/in/om-pawar-abb45b422)

</div>
