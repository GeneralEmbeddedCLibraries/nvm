# Changelog
All notable changes to this project/module will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project/module adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

---
## V2.1.0 - xx.xx.2023

### Added 
 - Added eeprom emulation for flash memory devices

### Fixed
 - Invalid address & size checking for read, write and erase API function

### Todo
- Configuration table checker

---
## V2.0.0 - 14.12.2022

### Added 
 - Detailed description of module

### Changed
 - API change:
    + Added new API function: *nvm_deinit*
    + Change function prototype of *nvm_is_init*
 - Memory driver interface must contain also de-init function

### Fixed
 - Missing check for initialization flag
 - Removed assert from init function. Now enables multiple call of init function

---
## V1.0.1 - 25.07.2021

### Added
 - Copyright notice

---
## V1.0.0 - 26.06.2021

### Added
 - Single configuration table for definition of NVM regions
 - Write/Read/Erase functionalities based on region
 - Each region has its assigned low level memory driver
 - Multiple access protection 

---