# 💡 Ws2812Universal

## Bibliothèque WS2812 universelle pour ESP32 — Architecture Chemins de Fer du Chablais (CFC)

**Ws2812Universal** est une bibliothèque C++ modulaire conçue pour les projets embarqués basés sur **ESP32**, structurée exactement comme **CanUniversal**.

Elle fournit une abstraction unifiée permettant de :

- piloter un ou plusieurs bus de LED WS2812 (3 LED sur le convertisseur, 9 sur le hub — le nombre est libre et fourni par la configuration)
- émettre par le périphérique **RMT matériel**, de façon **non bloquante**
- **aucun bit-banging, aucune interruption coupée** — le DCC tourne en parallèle
- choisir le **canal RMT** de chaque bus pour cohabiter avec ceux du DCC
- fournir la configuration matérielle par l'application (Provider)

Cette bibliothèque est utilisée dans l'architecture Chemins de Fer du Chablais (CFC) : convertisseur DCC_CAN_RMT-Booster, DCC_HUB_DISTRIB-Booster, CAN_DCC_RMT-Booster_DRV8874 et Extension_CantonController (EXCC).

# ✨ Fonctionnalités principales

## 🔧 Initialisation automatique (Ws2812Init)

- Initialisation centralisée de *tous* les bus déclarés par l'application
- Support **multi-bus** : WS2812\_0, WS2812\_1, WS2812\_2…
- Tolérante aux erreurs : un bus absent ou mal câblé est marqué **invalide**, sans jamais faire crasher l'application
- Retourne toujours `true` : les erreurs sont gérées bus par bus

---

## 📡 Gestion du bus (Ws2812Bus)

- Émission RMT **non bloquante** : `show()` copie la trame dans le ring buffer du driver et rend la main immédiatement
- Pas de bit-banging, aucune interruption désactivée
- Accès sécurisé : `Ws2812Bus::bus(index)` renvoie toujours une référence valide
- Rythme libre : `busy()` (sonde non bloquante) et `waitDone()` si nécessaire
- ⚠️ **Cohabitation DCC** : le canal RMT de chaque bus est choisi par l'application et doit être distinct de ceux utilisés par le DCC

---

## 🆕 Nouveautés v1.1.0

### 🔆 Luminosité à chaud (`setBrightness`)

La luminosité n'est plus « cuite » dans le framebuffer à l'écriture : elle est appliquée **à l'encodage RMT**, au moment du `show()`. Conséquence : `setBrightness()` est réglable à tout moment, effet visible à l'émission suivante, sans réécrire le buffer.

### 🎨 `WsCrgb` — couleur compatible `CRGB` (FastLED)

Structure RGB 3 octets (`r`, `g`, `b`) **binaire-compatible avec le `CRGB` de FastLED** : un tableau `WsCrgb strip[9]` se lit comme un `CRGB strip[9]`. Couleurs nommées via API statique (`WsCrgb::Red()`, `Green()`, `Orange()`…) et `ws_sin8()`, port exact du `sin8` FastLED pour les animations.

### 🚏 `Ws2812All` — opérations globales

`showAll()` (émet sur tous les bus valides, jamais bloquant), `setBrightnessAll()`, `droppedShows()` (diagnostic : shows ignorés parce qu'un bus était occupé).

### 🧱 Buffer externe zéro copie (`extRgb888`)

La config peut pointer vers un buffer `{R,G,B}×numLeds` **possédé par l'application** : la lib l'utilise tel quel (style FastLED), sans allocation ni copie.

### 🎭 Façade `Ws2812FastLED` — migration FastLED en douceur

```cpp
#include <Ws2812Universal.h>

static WsCrgb stripFeux[9];   // buffer de l'application

void setup() {
    // pin + canal RMT EXPLICITES + buffer externe : zéro copie
    Ws2812FastLED::addLeds(PIN_FEUX, RMT_CHANNEL_0, stripFeux, 9);
    Ws2812FastLED::setBrightness(128);
    Ws2812FastLED::clear(true);   // efface + émet
}

void loop() {
    stripFeux[0] = WsCrgb::Green();   // écriture directe, comme CRGB
    Ws2812FastLED::show();            // tous les strips, non bloquant
}
```

Aucune allocation de canal cachée : chaque strip reçoit **son canal RMT explicitement** — c'est l'application qui décide de la carte des canaux.

---

## 🧩 Configuration modulaire (Ws2812ConfigProvider)

Chaque module fournit sa propre configuration matérielle (pin, nombre de LED, canal RMT, luminosité, buffer externe optionnel).

➡️ **Aucun code matériel dans la bibliothèque**, tout est externalisé dans les modules.

---

## 🗺️ Exemple d'allocation des canaux RMT (EXCC)

| Canal RMT | Usage |
|---|---|
| 0 – 6 | 7 strips WS2812 (mâts H/AH, œillets, directions, status) |
| 7 | DCC régénéré (DccRegen, DRV8874) |

L'attribution vit dans `EXCC_Pins.h` — la bibliothèque ne choisit rien toute seule.

---

## 🧠 Architecture propre

- Miroir strict de CanUniversal : mêmes patterns, même style, mêmes conventions
- Séparation stricte entre la bibliothèque générique et les modules spécifiques
- Timing WS2812 conforme (tick 25 ns) : bit 0 = 300/700 ns, bit 1 = 700/300 ns, reset ≥ 50 µs

## 📦 Installation

### Via PlatformIO (recommandé)

```ini
lib_deps =
    prune74/Ws2812Universal
```

### Via l'IDE Arduino

Ajoutez ce dépôt via le Gestionnaire de bibliothèque ou copiez-le dans votre dossier `libraries/`.

## 🚀 Utilisation (API historique, v1.0 conservée)

```cpp
#include <Ws2812Universal.h>

// Configuration fournie par le module (pattern Provider)
class MyConfig : public Ws2812ConfigProvider {
    const Ws2812BusConfig bus0 { true, GPIO_NUM_18, 3, RMT_CHANNEL_7, 255 };
public:
    uint8_t busCount() const override { return 1; }
    const Ws2812BusConfig &busConfig(uint8_t) const override { return bus0; }
};

MyConfig config;

void setup() {
    Ws2812Init::begin(config);          // tolérant aux erreurs
}

void loop() {
    Ws2812Bus &bus = Ws2812Bus::bus(0);
    bus.fill(0x200000);
    bus.show();                          // NON bloquant
    delay(1000);
}
```

## 📁 Structure du projet

```
Ws2812Universal/
 ├── src/
 │    ├── Ws2812Bus.cpp / .h        ← émission RMT, buffer interne ou externe
 │    ├── Ws2812Config.h             ← config + Provider (v1.1.0 : extRgb888)
 │    ├── Ws2812Init.cpp / .h        ← initialisation tolérante
 │    ├── Ws2812All.cpp / .h         ← v1.1.0 : showAll / setBrightnessAll / droppedShows
 │    ├── WsCrgb.cpp / .h            ← v1.1.0 : couleur compatible CRGB + ws_sin8
 │    ├── Ws2812FastLED.cpp / .h     ← v1.1.0 : façade FastLED-like
 │    └── Ws2812Universal.h          ← en-tête principal
 ├── Examples/
 │    ├── BasicExample/             ← API historique (non-régression)
 │    ├── HubExample/               ← multi-bus, luminosité à chaud
 │    └── FastLEDCompatExample/     ← façade Ws2812FastLED + WsCrgb
 ├── library.json / library.properties / keywords.txt / LICENSE.txt / README.md
```

## 🎯 Compatibilité

- **Cible** : ESP32, arduino-esp32 2.0.x (`espressif32@6.9.0`), driver RMT legacy
- **Usage** : nombre de LED libre (3 sur le convertisseur, 9 sur le hub, toute autre valeur acceptée)

## 📜 Licence

MIT — voir .