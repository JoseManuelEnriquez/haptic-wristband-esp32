# 🦾 Pulsera de Vibración Háptica

> Componente de hardware para un sistema de asistencia a personas con discapacidad visual. Desarrollado como complemento al TFG *Sistema de Asistencia para Personas Ciegas*.

---

## 📖 Descripción

Este repositorio contiene el firmware de una **pulsera de vibración háptica** diseñada para proporcionar retroalimentación táctil en tiempo real. La pulsera actúa como el componente de salida de un sistema mayor de asistencia a la navegación para personas ciegas, traduciendo señales del sistema principal en patrones de vibración perceptibles por el usuario.

El firmware está desarrollado sobre **ESP32** usando el framework **ESP-IDF**, con una arquitectura basada en componentes que facilita su integración con el sistema central. La comunicación con el sistema principal se realiza vía **Bluetooth Low Energy (BLE)**, y la gestión concurrente de tareas se apoya en **FreeRTOS**.

---

## ✨ Características

- Comunicación inalámbrica con el sistema principal mediante **BLE** (Bluetooth Low Energy)
- Gestión concurrente de tareas con **FreeRTOS**
- Componente `HapticController` reutilizable e independiente
- Gestión de patrones de vibración configurables
- Bajo consumo energético orientado a uso portátil
- Compilación mediante CMake + ESP-IDF

---

## 🛠️ Tecnologías

| Tecnología | Uso |
|---|---|
| ESP32 | Microcontrolador de la pulsera |
| ESP-IDF | Framework de desarrollo |
| FreeRTOS | Gestión de tareas concurrentes |
| BLE (Bluetooth Low Energy) | Comunicación inalámbrica con el sistema principal |
| C++ | Lenguaje principal del firmware |
| CMake | Sistema de compilación |

---

## 📁 Estructura del proyecto

```
pulsera-vibracion/
├── components/
│   └── HapticController/   # Componente de control háptico
├── main/                   # Punto de entrada del firmware
├── CMakeLists.txt          # Configuración de compilación
├── .devcontainer/          # Entorno de desarrollo en contenedor
└── .vscode/                # Configuración del editor
```

---

## 🚀 Requisitos previos

- [ESP-IDF v5.x](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/get-started/) instalado y configurado (`IDF_PATH` definido en el entorno)
- CMake ≥ 3.22
- Placa ESP32

---

## ⚙️ Compilación y flasheo

```bash
# Configurar el entorno ESP-IDF
. $IDF_PATH/export.sh

# Compilar el proyecto
idf.py build

# Flashear a la placa (ajustar el puerto según corresponda)
idf.py -p /dev/ttyUSB0 flash

# Monitorizar la salida serie
idf.py -p /dev/ttyUSB0 monitor
```

También puedes usar el **Dev Container** incluido en `.devcontainer/` para tener el entorno ESP-IDF ya configurado sin instalaciones adicionales.

---

## 🔗 Contexto del proyecto

Este repositorio es un submódulo de hardware del proyecto principal **Sistema de Asistencia para Personas Ciegas** (TFG), que integra visión por computador, procesamiento de señales y retroalimentación háptica para ayudar a personas con discapacidad visual en su navegación cotidiana.

---

## 📄 Licencia
Este proyecto es de uso académico y forma parte de un Trabajo de Fin de Grado. Si deseas reutilizarlo o adaptarlo, por favor contacta conmigo primero.
