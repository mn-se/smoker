# Schematics and Circuit Diagrams

This file contains conceptual Mermaid diagrams for the Smoker project.

> These diagrams are for system-level understanding and prototyping. Any real mains wiring for the electric stove should be handled with appropriate safety precautions and local electrical standards.

## 1. System Overview

```mermaid
%%{init: {'flowchart': {'htmlLabels': true}}}%%
flowchart LR
    User["User"] -->|Set target / monitor| Smartphone["Smartphone / Browser"]
    Smartphone -->|Wi-Fi / Web UI| Core2["M5Stack Core2<br/>ESP32 + display"]
    Core2 -->|SPI| Sensor["MAX6675<br/>Thermocouple sensor"]
    Sensor -->|Temperature feedback| Core2
    Core2 -->|Smart plug control| Plug["Meross Smart Plug"]
    Plug -->|Power switching| Stove["Electric stove<br/>SK-65V"]
    Stove -->|Heat output| Sensor
    Core2 -->|Shows current temp<br/>and heater status| Display["Core2 screen"]
    Display -->|Visible to user| User
```

## 2. Conceptual Wiring Diagram

```mermaid
%%{init: {'flowchart': {'htmlLabels': true}}}%%
flowchart TD
    User["User"] -->|Operates| Smartphone["Smartphone"]

    subgraph Core2["M5Stack Core2"]
        MCU["ESP32 MCU"]
        Display["Built-in screen"]
        VCC["3.3V"]
        GND["GND"]
        CS["GPIO / CS"]
        SCK["GPIO / SCK"]
        SO["GPIO / SO"]
    end

    subgraph SensorBlock["MAX6675 Module"]
        MAX6675["MAX6675 Thermocouple Interface"]
    end

    subgraph PlugBlock["Meross Smart Plug"]
        Relay["Relay / Smart Switch"]
    end

    subgraph StoveBlock["Electric Stove"]
        Heater["SK-65V Heater"]
    end

    Smartphone -->|Wi-Fi| MCU
    Display -->|Shows temp / heater state| User

    VCC --> MAX6675
    GND --> MAX6675
    CS --> MAX6675
    SCK --> MAX6675
    SO --> MAX6675

    MCU -->|SPI| MAX6675
    MCU -->|Wi-Fi command| Relay
    Relay --> Heater
```

## 3. Control Flow

```mermaid
sequenceDiagram
    participant User as Smartphone User
    participant Core2 as M5Stack Core2
    participant Sensor as MAX6675
    participant Plug as Meross Plug
    participant Stove as Electric Stove

    User->>Core2: Set target temperature
    Core2->>Sensor: Read current temperature
    Sensor-->>Core2: Temperature value
    Core2->>Core2: Calculate PID control
    Core2->>Plug: Turn heater ON/OFF
    Plug->>Stove: Supply or cut power
    Stove-->>Sensor: Heat source changes temperature
    Sensor-->>Core2: Updated temperature
    Core2-->>User: Show current status on screen / web UI
```
