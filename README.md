# ChessLab ⚡ — Architecture Projet

---

## Architecture Système

```
┌─────────────────────────────────────────────────────┐
│                    ESP32-C6                          │
│  WiFi 6 · BLE 5 · RISC-V 160MHz · 512KB SRAM       │
└──────┬──────────────┬───────────────┬───────────────┘
       │              │               │
  Hall Sensors    RGB LEDs        Power Mgmt
  (8x8 = 64)     (8x8 = 64)      18650 Pack
```

---

## HARDWARE

### Capteurs Hall — **A3144**
- A3144 = digital → simple présence/absence
- **Choix : A3144** pour simplicité et coût

### LEDs RGB — **WS2812B** (NeoPixel)
- Adressables individuellement
- 1 fil de data pour les 64 LEDs en série (chaîne serpentine)
- 5V, ~60mA/LED max → faudra un bon power management

### Multiplexage Hall Sensors
64 capteurs analogiques → trop de pins pour l'ESP32-C6  
**Solution : Multiplexeurs 74HC4051 (8:1 analog mux)**
- 8 mux × 8 capteurs = 64 capteurs
- Seulement ~11 pins ESP32 nécessaires

### Power — 18650
- **2× 18650 en parallèle** → ~6000mAh
- **TP4056** comme IC de charge/gestion
- **Régulateurs :**
  - 5V/3A pour les WS2812B (buck converter MP3608)
  - 3.3V pour ESP32 + Hall (LDO AMS1117-3.3)

---

## Layout PCB

### Routing challenge
- **Serpentine WS2812B** : DATA IN → DATA OUT case par case
- **Matrices Hall** : lignes/colonnes pour mux
- **Épaisseur PCB** : 1.6mm standard
- **Couche diffusante** : à imprimer en 3d

---

## Aimants dans les pièces
- **N52 cylindrique Ø6×3mm** dans la base des pièces
- Colle epoxy dans un logement imprimé 3D

---

## 🖥️ Firmware ESP32-C6 — Stack logiciel

\```
FreeRTOS
├── Task: Hall Scanner (polling 50ms, 64 sensors)
├── Task: LED Controller (état échiquier → animation)
├── Task: Chess Engine (position légale, coups valides)
├── Task: BLE/WiFi (app mobile / web interface)
└── Task: Battery Monitor (ADC sur voltage divider)
\```

**Libs utiles :**
- `FastLED` ou `Adafruit NeoPixel` pour WS2812B
- `Stockfish` (version allégée) via WASM si WebSerial
- Interface web servie depuis SPIFFS/LittleFS

---

## 📦 Boîtier 3D — Contraintes

Total hauteur estimée : **~25-30mm**

---

## 🗺️ Roadmap 

| Phase | Description |
|-------|-------------|
| **P1** | Prototype 1 case (1 LED + 1 Hall + mux) sur breadboard |
| **P2** | PCB 4×4 test (KiCad) — valider le multiplexage |
| **P3** | PCB 8×8 full + boîtier v1 |
| **P4** | Firmware chess engine + app |
| **P5** | Impression 3D boîtier final + pièces modifiées |

## Fonctionnement Allumage pas à pas
*Allumage :*
1. Appui BTN_PWR
2. Gate du MOSFET passe à GND → MOSFET conduit
3. VSYS s'allume → ESP32 démarre
4. ESP32 maintient le GPIO à LOW → garde le MOSFET ouvert
5. relâches bouton → le système reste allumé

*Extinction logicielle (soft power off) :*
1. ESP32 détecte inactivité (ex: 10 min sans coup joué)
2. Sauvegarde état si besoin
3. GPIO passe à HIGH → MOSFET se bloque → tout s'éteint

*Extinction manuelle :*
1. Appui long BTN_PWR (ex: 3 secondes)
2. ESP32 détecte le GPIO d'entrée du bouton
3. Même séquence que soft power off


```
Gestion de l'allumage — MOSFET P-Channel AO3401

VBAT
 │
 R1 10kΩ
 │
 ├─────────────────────────► Gate AO3401 (P-MOS)
 │                                │
 │         ┌──── R2 100kΩ ────────┘
 │         │
 │      Source AO3401
 │         │ = VBAT
 │         │
 │        Drain ──────────────────► VSYS
 │                                   │
 BTN_PWR                        (MT3608 + AMS1117)
 │                                   │
 └──── vers Gate              ESP32-C6
                                │         │
                          GPIO_PWR_HOLD  GPIO_BTN_PWR
                          (maintien ON)  (lecture bouton)
```

## Répartition des composants par pcb

```
MAIN BOARD :                    POWER BOARD :
├── ESP32-C6                    ├── TP4056
├── USB-C connector             ├── DW01A + FS8205A
├── 64× WS2812B                 ├── 18650 connector
├── 64× A3144                   ├── MT3608 (5V)
├── 8× 74HC4051                 ├── AMS1117 (3.3V)
├── 1× 74HC138                  ├── MOSFET power switch
├── BTN_BOOT + BTN_RESET        ├── BTN_PWR + LED power
└── JST vers power board        └── JST vers main board
```

Connecteurs entre les cartes :
- **JST 4-pin** pour l'alimentation:
  - Pin 1 : VBAT (18650+)
  - Pin 2 : GND
  - Pin 3 : 5V (MT3608 output vers main board)
  - Pin 4 : 3.3V (AMS1117 output vers main board)

## Pinout ESP32
13 -> GPIO12 = D-
14 -> GPIO13 = D+
8  -> GPIO0  = S0 (mux sélection canal)
9  -> GPIO1  = S1
27  -> GPIO2  = S2
26  -> GPIO3  = A0 (74HC138 sélection rangée)
25 -> GPIO16  = A1
24 -> GPIO17  = A2
16 -> GPIO18  = SIG_ADC (lecture capteurs Hall)
17 -> GPIO19  = DATA WS2812B (via 33Ω)
18 -> GPIO20  = VBAT
15 -> GPIO9  = BOOT button (déjà prévu)
19 -> GPIO21 = BTN_PWR (lecture bouton power)
20 -> GPIO22  = PWR_HOLD (maintien alimentation)

Signal (données, GPIO)  : 0.25mm
Alimentation 3.3V       : 0.5mm
Alimentation 5V LEDs    : 1.0mm
VBAT                    : 1.0mm
VSYS                    : 1.0mm
USB D+ / D-             : 0.2mm  

Via standard :
→ Drill    : 0.4mm
→ Pad      : 0.8mm

Via power :
→ Drill    : 0.6mm
→ Pad      : 1.2mm


