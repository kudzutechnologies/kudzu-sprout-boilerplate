# Peripherals Directory

This directory contains the ported or generated code for interfacing with device peripherals over the kernel interfaces.

## Description

Each `Peripheral` library exposes a set of static functions that can perform atomic operations to a peripheral unit, taking as an argument a kernel interface.

We are currently exposing the `I2CInterface`, implemented by the `ModuleI2CExpander` module.
