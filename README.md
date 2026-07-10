# ChessLab — Documentation technique

Échiquier connecté détectant la position des pièces (capteurs à effet Hall) et affichant un retour visuel case par case (LEDs RGB adressables), pour visualiser une partie et aider à l'apprentissage du jeu.

## Architecture système

```
                        ESP32-C6
        WiFi 6 · BLE 5 · RISC-V 160 MHz · 512 KB SRAM
                            |
        +-------------------+-------------------+
        |                   |                   |
  Capteurs Hall         LEDs RGB           Alimentation
   (8x8 = 64)          (8x8 = 64)          Pack 18650
```

## Hardware

### Capteurs de position — A3144

Capteur à effet Hall digital (sortie tout-ou-rien), suffisant pour détecter la présence ou l'absence d'une pièce sur une case. Choisi pour sa simplicité de mise en œuvre et son faible coût.

### LEDs — WS2812B (NeoPixel)

64 LEDs adressables individuellement, câblées en une seule chaîne série (une ligne de données, montage serpentin case par case). Alimentation 5 V, consommation crête d'environ 60 mA par LED.

### Multiplexage des capteurs Hall

64 capteurs analogiques dépassent le nombre de broches disponibles sur l'ESP32-C6. Solution retenue : multiplexeurs analogiques 74HC4051 (8 voies vers 1), soit 8 multiplexeurs pour 64 capteurs, ramenant le besoin à environ 11 broches ESP32.

### Alimentation — 18650

- 2 cellules 18650 montées en parallèle (~6000 mAh)
- Gestion de charge : IC TP4056
- Régulation :
  - 5 V / 3 A pour les LEDs WS2812B (convertisseur buck MP3608)
  - 3,3 V pour l'ESP32 et les capteurs Hall (LDO AMS1117-3.3)

## Circuit d'alimentation (mise sous tension)

Commutation assurée par un MOSFET P-Channel AO3401, piloté par le bouton d'alimentation et maintenu par l'ESP32 :

```
VBAT
 |
 R1 10k
 |
 +----------------------> Gate AO3401 (P-MOS)
 |                              |
 |        +---- R2 100k --------+
 |        |
 |     Source AO3401 = VBAT
 |        |
 |       Drain --------------------> VSYS
 |                                     |
BTN_PWR                          (MT3608 + AMS1117)
 |                                     |
 +---- vers Gate                 ESP32-C6
                                   |         |
                          GPIO_PWR_HOLD  GPIO_BTN_PWR
                          (maintien ON)  (lecture bouton)
```

**Séquence d'allumage**
1. Appui sur BTN_PWR
2. La gate du MOSFET passe à GND, le MOSFET conduit
3. VSYS s'active, l'ESP32 démarre
4. L'ESP32 maintient son GPIO à l'état bas pour garder le MOSFET conducteur
5. Le bouton peut être relâché, le système reste alimenté

**Extinction logicielle**
1. Inactivité détectée (par exemple 10 minutes sans coup joué)
2. Sauvegarde de l'état de la partie si nécessaire
3. Le GPIO passe à l'état haut, le MOSFET se bloque et coupe l'alimentation

**Extinction manuelle**
1. Appui long sur BTN_PWR (environ 3 secondes)
2. Détection par l'ESP32 via le GPIO de lecture du bouton
3. Même séquence que l'extinction logicielle

## Répartition des composants entre les deux cartes

| Carte principale | Carte d'alimentation |
|---|---|
| ESP32-C6 | TP4056 |
| Connecteur USB-C | DW01A + FS8205A |
| 64× WS2812B | Connecteur 18650 |
| 64× A3144 | MT3608 (5 V) |
| 8× 74HC4051 | AMS1117 (3,3 V) |
| 1× 74HC138 | Commutation MOSFET |
| BTN_BOOT + BTN_RESET | BTN_PWR + LED power |
| Connecteur JST vers carte d'alimentation | Connecteur JST vers carte principale |

Les deux cartes sont reliées par un connecteur JST 4 broches :

| Broche | Signal |
|---|---|
| 1 | VBAT (18650+) |
| 2 | GND |
| 3 | 5 V (sortie MT3608 vers carte principale) |
| 4 | 3,3 V (sortie AMS1117 vers carte principale) |

## Layout PCB

- Routage serpentin des WS2812B (entrée/sortie de données case par case)
- Matrices de capteurs Hall organisées en lignes/colonnes vers les multiplexeurs
- Épaisseur de PCB standard : 1,6 mm
- Couche diffusante imprimée en 3D au-dessus des LEDs

### Largeurs de piste

| Signal | Largeur |
|---|---|
| Données / GPIO | 0,25 mm |
| Alimentation 3,3 V | 0,5 mm |
| Alimentation 5 V (LEDs) | 1,0 mm |
| VBAT | 1,0 mm |
| VSYS | 1,0 mm |
| USB D+ / D- | 0,2 mm |

### Vias

| Type | Perçage | Pad |
|---|---|---|
| Standard | 0,4 mm | 0,8 mm |
| Puissance | 0,6 mm | 1,2 mm |

## Pinout ESP32-C6

| Broche | GPIO | Fonction |
|---|---|---|
| 13 | GPIO12 | USB D- |
| 14 | GPIO13 | USB D+ |
| 8 | GPIO0 | Mux — sélection canal S0 |
| 9 | GPIO1 | Mux — sélection canal S1 |
| 27 | GPIO2 | Mux — sélection canal S2 |
| 16 | GPIO18 | 74HC138 — sélection rangée A0 |
| 25 | GPIO16 | 74HC138 — sélection rangée A1 |
| 24 | GPIO17 | 74HC138 — sélection rangée A2 |
| 26 | GPIO3 | Lecture capteurs Hall (A_OUT) |
| 17 | GPIO19 | Données WS2812B (via résistance 33 Ω) |
| 18 | GPIO20 | VBAT |
| 15 | GPIO9 | Bouton BOOT |
| 19 | GPIO21 | Lecture bouton d'alimentation (BTN_PWR) |
| 20 | GPIO22 | Maintien alimentation (PWR_HOLD) |

## Fixation des pièces

Aimants cylindriques N52, Ø6×3 mm, insérés dans la base de chaque pièce et collés à l'époxy dans un logement imprimé en 3D. Ce choix permet une détection fiable par les capteurs Hall tout en restant invisible une fois la pièce posée.

## Boîtier

Hauteur totale estimée : 25 à 30 mm, dimensionnée pour loger la carte principale, la carte d'alimentation et les deux packs 18650.

## Firmware

Architecture logicielle sous FreeRTOS, organisée en tâches indépendantes :

| Tâche | Rôle |
|---|---|
| Hall Scanner | Scrutation des 64 capteurs (polling 50 ms) |
| LED Controller | Traduction de l'état de l'échiquier en animation LED |
| Chess Engine | Validation des positions et des coups légaux |
| BLE/WiFi | Communication avec l'application mobile ou l'interface web |
| Battery Monitor | Suivi de la tension batterie via diviseur de tension sur ADC |

**Bibliothèques envisagées**
- `FastLED` ou `Adafruit NeoPixel` pour le pilotage des WS2812B
- `Stockfish` (version allégée, via WASM) pour le moteur d'échecs
- Interface web servie depuis la mémoire flash (SPIFFS/LittleFS)

## Feuille de route

| Phase | Objectif |
|---|---|
| P1 | Prototype d'une case (1 LED + 1 capteur Hall + multiplexeur) sur breadboard |
| P2 | PCB de test 4×4 (KiCad) pour valider le multiplexage |
| P3 | PCB complet 8×8 et boîtier v1 |
| P4 | Firmware — moteur d'échecs et application |
| P5 | Impression 3D du boîtier final et des pièces modifiées |
