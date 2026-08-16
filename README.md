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

| Peripheral | Signal | Pin(s) |
|---|---|---|
| 🖥️ LCD (16×2, 4-bit) | RS / EN / D4–D7 | P0.16 / P0.17 / P0.18–P0.21 |
| 🖧 UART0 (debug/log) | TXD0 / RXD0 | P0.0 / P0.1 |
| 💾 I2C0 (EEPROM) | SCL0 / SDA0 | P0.2 / P0.3 |
| 🛡️ Tamper switch | Active LOW | P0.4 |
| 🧑‍💻 Admin button | EINT2, falling edge | P0.7 |
| 📶 UART1 (HC-05) | TXD1 / RXD1 | P0.8 / P0.9 |
| 🔢 Keypad (4×4) | Rows / Cols | P1.16–19 / P1.20–23 |
| ⚙️ Motor (L293D) | IN1 / IN2 | P1.24 / P1.25 |
| 🔊 Buzzer | Signal | P1.26 |

**Core clock:** 60 MHz (12 MHz crystal, PLL M=5 / P=2) · **PCLK:** 15 MHz

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

## 🏗️ Software Architecture
<img src="Hardware Image/Project Workflow/IMG-20260806-WA0011.jpg" alt="Photo" width="500">

---

## 🔌 Hardware Block Diagram
<img src="Hardware Image/Project Workflow/BlockDiagram.jpg" alt="Photo" width="500">

---

## 🔄 System Workflow

<img src="Hardware Image/Project Workflow/IMG-20260806-WA0010.jpg" alt="Photo" width="500">

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
