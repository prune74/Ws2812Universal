# 💡 Ws2812Universal

## Bibliothèque WS2812 universelle pour ESP32 — Architecture Chemins de Fer du Chablais (CFC)

**Ws2812Universal** est une bibliothèque C++ modulaire conçue pour les projets embarqués basés sur **ESP32**, structurée exactement comme **CanUniversal**.

Elle fournit une abstraction unifiée permettant de :

- piloter un ou plusieurs bus de LED WS2812 (3 LED sur le convertisseur, 9 sur le hub — le nombre est libre et fourni par la configuration)
- émettre par le périphérique **RMT matériel**, de façon **non bloquante**
- **aucun bit-banging, aucune interruption coupée** — le DCC tourne en parallèle
- choisir le **canal RMT** de chaque bus pour cohabiter avec ceux du DCC
- fournir la configuration matérielle par l'application (Provider)

Cette bibliothèque est utilisée dans :

- l'architecture Chemins de Fer du Chablais (CFC).

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

## 🧩 Configuration modulaire (Ws2812ConfigProvider)

Chaque module fournit sa propre configuration matérielle (pin, nombre de LED, canal RMT, luminosité).

➡️ **Aucun code matériel dans la bibliothèque**, tout est externalisé dans les modules.

---

## 🧠 Architecture propre

- Miroir strict de CanUniversal : mêmes patterns, même style, mêmes conventions
- Séparation stricte entre la bibliothèque générique et les modules spécifiques
- Timing WS2812 conforme (tick 25 ns) : bit 0 = 300/700 ns, bit 1 = 700/300 ns, reset ≥ 50 µs

## 📦 Installation

### Via PlatformIO (recommandé)

Dans votre `platformio.ini` :

```ini
lib_deps =
    prune74/Ws2812Universal
```

### Via l'IDE Arduino

Ajoutez ce dépôt via le Gestionnaire de bibliothèque ou copiez-le dans votre dossier `libraries/`.

## 🚀 Utilisation

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
 │    ├── Ws2812Bus.cpp
 │    ├── Ws2812Bus.h
 │    ├── Ws2812Config.h
 │    ├── Ws2812Init.cpp
 │    ├── Ws2812Init.h
 │    └── Ws2812Universal.h      ← en-tête principal
 ├── examples/
 │    ├── BasicExample/
 │    │    └── BasicExample.ino
 │    └── HubExample/
 │         └── HubExample.ino
 ├── library.json                ← registre PlatformIO
 ├── library.properties          ← IDE Arduino
 ├── keywords.txt
 ├── LICENSE.txt
 └── README.md
```

## 🎯 Compatibilité

- **Cible** : ESP32, arduino-esp32 2.0.x (`espressif32@6.9.0`)
- **Usage** : nombre de LED libre (3 sur le convertisseur, 9 sur le hub, toute autre valeur acceptée)

## 📜 Licence

MIT — voir [LICENSE.txt](LICENSE.txt).