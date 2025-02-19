# Temperaturmåler
## Indholdsfortegnelse
1. [Beskrivelse](#beskrivelse)
2. [Filoversigt](#filoversigt)
3. [Brugervejledning](#brugervejledning)
    1. [Setup](#setup)
    2. [Brug](#brug)
4. [Dokumentation/Demo](#dokumentationdemo)
    1. [Fritzing diagram](#fritzing-diagram)

## Beskrivelse
Dette projekt er en temperaturmåler, som måler temperaturen og viser den på en hjemmeside(som også kører på esp32). Den er bygget med en ESP32, en DS18B20 temperatursensor. Den måler temperaturen hvert 5. minute og viser den på en hjemmeside. Hjemmesiden holds automatisk opdateres.

## Filoversigt
* `src/` - Kildekode
* `src/main.cpp` - Entry point for programmet
* `src/preferences_handler.h` - Utility fil til at samle funktioner til at gemme og hente preferences
* `src/storage_handler.h` - Utility fil til at samle functioner til at gemme og hente data fra SPIFFS
* `src/webserver_setup.h` - Utility fil til at samle funktione til at opsætte webserveren
* `src/websocket_handler.h` - Utility fil til at samle funktioner til at håndtere websockets
* `data/` - Data til SPIFFS
* `data/ap_root.html` - Root html filen til ap mode
* `data/sta_root.html` - Root html filen til standard mode
* `data/sta_script.js` - Javascript fil til standard mode


# Brugervejledning

### Setup
* Hold reset knappen nede til indbygged led blinker
* Derefter forbind til access pointet `ESP32-Felix` med password `password`
* Åben en browser og gå til `192.168.4.1``
* Indtast dit eget netværks SSID og password
* Tryk på `Save` og vent på at enheden genstarter


### Brug
* Efter enheden er forbundet til dit eget netværk, kan du finde IP adressen seriel monitor eller på din router
* Gå til IP adressen i en browser
* Her kan du se en graf over temperatur
* Du kan også se den nuværende temperatur

# Dokumentation/Demo

## Fritzing diagram
![Fritzing Diagram](docs/fritzing_diagram.png)

## Demo
[![Demo video](https://img.youtube.com/vi/LTltwZoyWiU/0.jpg)](https://www.youtube.com/watch?v=LTltwZoyWiU)