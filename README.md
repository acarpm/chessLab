# ChessLab ⚡ — Architecture Projet

---

## 🏗️ Architecture Système

\```
┌─────────────────────────────────────────────────────┐
│                    ESP32-C6                          │
│  WiFi 6 · BLE 5 · RISC-V 160MHz · 512KB SRAM       │
└──────┬──────────────┬───────────────┬───────────────┘
       │              │               │
  Hall Sensors    RGB LEDs        Power Mgmt
  (8x8 = 64)     (8x8 = 64)      18650 Pack
\```

---

## 🔌 Stack Hardware Recommandé

### Capteurs Hall — **SS49E** ou **A3144**
- SS49E = analogique → détection fine de la force du champ (meilleur pour distinguer pièces blanches/noires selon polarité aimant)
- A3144 = digital → simple présence/absence
- **Choix : SS49E** pour plus de richesse

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
- **IP5306** comme IC de charge/gestion
- **Régulateurs :**
  - 5V/3A pour les WS2812B (buck converter MP1584)
  - 3.3V pour ESP32 + Hall (LDO AMS1117-3.3)

---

## 📐 Layout PCB — Réflexions Clés

### Routing challenge
- **Serpentine WS2812B** : DATA IN → DATA OUT case par case
- **Matrices Hall** : lignes/colonnes pour mux
- **Épaisseur PCB** : 1.6mm standard
- **Couche diffusante** : acrylique dépoli 2mm sur le PCB

---

## 🧲 Aimants dans les pièces
- **N52 cylindrique Ø6×3mm** dans la base des pièces
- Polarité alternée blanc/noir → SS49E peut distinguer les 2 couleurs !
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

