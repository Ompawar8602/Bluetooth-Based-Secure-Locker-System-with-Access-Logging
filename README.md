<div align="center">

# 🔒Bluetooth-Based Secure Locker System 
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

<img src="Hardware Image/Project Workflow/IMG-20260806-WA0010.jpg" alt="Photo" width="500">

## 🔍 How It Works — Quick Walkthrough

1️⃣ **Power On** — LPC2148 initializes all peripherals (LCD, UART, I2C, RTC, keypad, motor, buzzer). On first-ever boot, default passwords (`1234` BT / `5678` keypad) are written to EEPROM.

2️⃣ **Standby** — LCD shows a welcome screen; system waits for a Bluetooth password, admin button press, or tamper/alarm event.

3️⃣ **Level-1 (Bluetooth)** — User sends a 4-digit password via HC-05 (e.g. `1234#`). Wrong → denied, logged, buzzer alert. Correct → proceeds to Level-2.

4️⃣ **Level-2 (Keypad)** — User enters a second password on the keypad (masked with `*`). Wrong → same denial flow. Correct → access granted.

5️⃣ **Unlock** — Motor (via L293D) opens the locker.

6️⃣ **Auto-Lock** — After a short hold, the motor reverses and re-locks automatically.

7️⃣ **Logging** — Every event (logins, denials, locker open/close, tamper, admin actions) is timestamped via RTC and streamed over UART0 to a PC.

8️⃣ **Tamper Detection** — Runs continuously in the background; any unauthorized enclosure access triggers an instant alert, regardless of system state.

9️⃣ **Admin Mode** — Admin button opens an on-device menu to set the clock, configure the alarm, or change passwords. Auto-exits after 15s idle.

🔟 **Alarm** — If configured, buzzer + LCD alert fire when RTC time matches the alarm time; stopped only via the admin menu.

---
##  Software Architecture 

<img src="Hardware Image/Project Workflow/IMG-20260806-WA0011.jpg" alt="Photo" width="500">

## 🧩 Software Architecture

The firmware is split into layered modules, each with one job — easier to debug and extend than a single large file.

- **Application Layer** (`projectmain.c`) — initializes peripherals and runs the main loop that sequences the two-factor auth flow, motor control, tamper checks, and admin/alarm handling.
- **Logic Layer** (`security.c`, `menu.c`) — tamper detection, default password setup, and event logging (`security.c`); the full admin menu for clock, alarm, and password changes (`menu.c`).
- **Driver Layer** — one focused file per peripheral: `bluetooth.c`, `keypad.c`, `lcd.c`, `eeprom.c`, `rtc.c`, `motor.c`, `buzzer.c`, `uart.c`. Each only knows how to talk to its own hardware.

This separation means swapping a peripheral (e.g. the LCD) only touches its driver file — the rest of the codebase stays untouched.

---

## 🔐 Security Architecture

Security is layered, not a single check:

- **Two-Factor Design** — Bluetooth password + physical keypad password; compromising one alone isn't enough.
- **Persistent Storage** — Passwords live in EEPROM (I2C), not memory, so they survive power loss and are only changeable via the admin menu.
- **Tamper Detection** — A dedicated switch is monitored continuously and independently of login state; any unauthorized enclosure access triggers an instant alert.
- **Full Audit Trail** — Every event (logins, denials, tamper, admin actions) is RTC-timestamped and logged over UART.
- **Fail-Safe Defaults** — A blank/corrupted EEPROM is auto-detected and re-initialized with safe factory defaults, never left in an undefined state.

---

## 📡 Communication Protocol

- **Bluetooth (Level-1)** — HC-05 over UART1 @ 9600 baud. Password sent as digits + `#` terminator (e.g. `1234#`); received via interrupt into a ring buffer so no bytes are lost.
- **Keypad (Level-2)** — Row/column matrix scanning; digits masked with `*` on the LCD as typed.
- **PC Audit Log (UART0)** — Separate channel streaming a live, human-readable log line per event, viewable on a laptop via USB-to-TTL.


---

## 🔌 Hardware Block Diagram

<img src="Hardware Image/Project Workflow/BlockDiagram.jpg" alt="Photo" width="500">

## 🧱 Hardware Architecture

Built around the **NXP LPC2148**, a 32-bit ARM7 microcontroller running at 60 MHz. All components connect to it through GPIO pins, each playing a specific role.

**Key Points:**

- ⚡ **Power Supply** — 12V DC input → stepped down to 5V (7805 regulator) → further stepped down to 3.3V for the microcontroller, protecting it from voltage noise/spikes.

- 📡 **UART0** — Streams a live, timestamped log of all system activity to a PC for real-time monitoring.

- 📶 **UART1** — Connects to the HC-05 Bluetooth module, receiving the wireless Level-1 password from a phone.

- 💾 **I2C0** — Connects to an EEPROM chip that stores both passwords permanently, so they survive power loss.

- 🔢 **4×4 Keypad** — Used for entering the Level-2 password and navigating the admin menu.

- 🛡️ **Tamper Switch** — Detects unauthorized physical access to the enclosure.

- 🧑‍💻 **Admin Button** — Instantly opens a settings menu (change passwords, set time, etc.) via a dedicated interrupt.

- 🖥️ **16×2 LCD** — Displays live status: welcome message, prompts, success/failure, and alerts.

- ⚙️ **DC Motor (via L293D)** — Physically locks and unlocks the locker.

- 🔊 **Buzzer** — Sounds alerts for tampering, wrong passwords, or alarms.

**Why This Design:**
- Two separate unlock methods (wireless + physical) mean an intruder needs both to break in — not just one.
- Every action is timestamped, so there's always a clear record of who accessed the locker and when.



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
