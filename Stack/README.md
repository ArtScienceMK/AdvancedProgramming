<a name="top"></a>

# Onegin Sorting

[![getting started](logo.png)](#-quickstart)

## Table of contents
- [About](#-about)
- [Quickstart](#-quickstart)
- [Dependencies](#-dependencies)
- [Building](#-building)
- [What's New](#-whats-new)
- [Documentation](#-documentation)
- [Contacts](#%EF%B8%8F-contacts)

## About

**Stack** V1.0 is an implementation of stack with several features:  

* Canary protection
* Detailed stack dump

## Quickstart

Download latest version from releases :) That's it!

## Dependencies

The list of dependencies:
1) g++ compiler 16.2.1 and newer

## Building

If you want to build this project on your machine follow this steps:

0) Check dependencies
1) ``` mkdir Stack ```
2) ``` cd Stack ```
3) ``` git clone https://github.com/ArtScienceMK/AdvancedProgramming/tree/main/Stack ```
4) ``` g++ main.cpp -o main ``` to disable any debug output

   ```  g++ main.cpp -DDEBUG -DSTACK_DEBUG -o main ``` to enable all debug output into console and activate asserts
5) ``` ./main ```

Congratulations!

## What's New

Stay tuned for new features!

## Documentation

POISON = element was not initialized
CANARY = value, stored in left and right bounds of stack buffer to check illegal memory access.

Canary protection:
First and last elements of buffer are canaries. They store CANARY value and being checked when StackVerify() called. 

Stack dump:
Prints out size, capacity and buffer of the stack (canaries and "poisonous" elements are being marked, moreover canaries
with incorrect value are being marked by BAD_CANARY).

## Contacts

If you will have any questions, suggestions and so on, just text me)

Mail: 55555artem.kulakov@gmail.com
