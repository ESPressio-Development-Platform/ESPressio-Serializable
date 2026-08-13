# ESPressio Serializable
Serializable Components of the Flowduino ESPressio Development Platform

Provides a templated implementation of Serializable Types and Type Handlers, which you can apply to any situation where you need to Serialize or Deserialize objects (or even commands - functions/methods) bound to a class, to and from JSON.

## Latest Stable Version
There is currently no stable released version.

## Compatibility

ESPressio Serializable is intended for modern Arduino-capable microcontrollers. ESP32 is the primary supported target. RP2040, SAMD, STM32, Renesas UNO R4, Teensy, and similar targets may be compatible when their Arduino core supplies the required C++ standard-library features and supports ArduinoJson 7.

The library uses ArduinoJson, ESPressio Tree, STL containers, and—within its thread-safe variants—`std::mutex` and `std::shared_mutex`. Classic AVR targets are not recommended for the complete library because those synchronization facilities may be unavailable and the library's dynamic-memory footprint may be impractical. Applications that use only non-thread-safe headers may support a wider set of targets, but must be compile-tested separately.

The broad manifest declarations describe the lack of processor-specific code, not guaranteed compatibility with every toolchain.

## ESPressio Development Platform
The **ESPressio** Development Platform is a collection of discrete (sometimes intra-connected) Component Libraries developed with a particular development ethos in mind.

The key objectives of the ESPressio Development Platform are:
- **Light-weight** - The Components should always strive to optimize memory consumption and operational overhead as much as possible, but not to the detriment of...
- **Ease of Use** - Many of our components serve as Developer-Friendly Abstractions of existing procedural code libraries.
- **Object-Oriented** - A `type` for everything, and everything in a `type`!
- **SOLID**:
- -  > **S**ingle Responsibility Principle (SRP)
    Break your code into smaller, focused components.
- - > **O**pen/Closed Principle (OCP)
    Be open for extension but closed for modification.
- - > **L**iskov Substitution Principle (LSP)
    Be substitutable for the base type without altering correctness.
- - > **I**nterface Segregation Principle (ISP)
    Break interfaces into specific, client-focused ones.
- - > **D**ependency Inversion Principle (DIP)
    Be dependent on abstractions, not concretions.

To the maximum extent possible within the limitations/restrictons/constraints of the C++ langauge, the Arduino platform, and Microcontroller Programming itself, all Component Libraries of the **ESPressio** Development Platform must strive to honour the **SOLID** principles.

## License
ESPressio (and its component libraries, including this one) are subject to the *Apache License 2.0*
Please see the [![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](LICENSE) accompanying this library for full details.

## Namespace
Every type/variable/constant/etc. related to *ESPressio* Serializable are located within the `Serializable` sub-namespace of the `ESPressio` parent namespace.

## Object Ownership

`PropertyFactory` and `CommandFactory` store non-owning references. The application
must keep every registered property or command alive until it is unregistered or
the corresponding factory is destroyed. Clearing or destroying a factory does not
delete registered objects.

Properties and commands cannot be copied or moved. Their names and, for
`StringProperty`, string values are owned by the object. String change callbacks
receive valid old and new value pointers for the duration of the callback; callers
must copy either string if it needs to be retained afterward.

## Platformio.ini
You can quickly and easily add this library to your project in PlatformIO by simply including the following in your `platformio.ini` file:

```ini
lib_deps = 
    https://github.com/Flowduino/ESPressio-Tree.git
    https://github.com/Flowduino/ESPressio-Serializable.git
```

Please note that this will use the very latest commits pushed into the repository, so volatility is possible.
This will of course be resolved when the first release version is tagged and published.
This section of the README will be updated concurrently with each release.
