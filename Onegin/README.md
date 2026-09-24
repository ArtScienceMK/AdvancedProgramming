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

**Onegin** V2.0 can help you to understand the Russian soul through the awesome poem "Eugene Onegin". Program can print a whole text of poem and sort it in any provided way:  

* Basic string comparison (like strcmp) 
* Letters-only comparison
* Each option comes with two opportunities: to sort in descending order, to process strings from right to left

## Quickstart

Download latest version from releases :) That's it!

## Dependencies

The list of dependencies:
1) g++ compiler 16.2.1 and newer

## Building

If you want to build this project on your machine follow this steps:

0) Check dependencies
1) ``` mkdir Onegin ```
2) ``` cd Onegin ```
3) ``` git clone https://github.com/ArtScienceMK/SquareSolver ```
4) ``` g++ main.cpp MyString.cpp -o main ``` to disable any debug output

   ```  g++ main.cpp MyString.cpp -DDEBUG -o main ``` to enable all debug output into console and activate asserts
5) ``` ./main ```

Congratulations!

## What's New

Added processing of command line arguments! Following flags are available:
* -i=[Path to file] Reading from file
* -o=[Path to file] Writing to file
* -h Showing a manual

## Documentation

List of sortings, available in program:
* Standard library QuickSort (qsort)
* Bubble sort, implemented by myself

List of comparators, available in program:
* CmpLettersAZ - compare strings from left to right only by letters (register independent), ignoring other symbols. Order of sorted strings is lexicographical. 
* CmpLettersZA - compare strings from left to right only by letters (register independent), ignoring other symbols. Order of sorted strings is reversed lexicographical. 
* CmpLettersInvAZ - compare strings from right to left only by letters (register independent), ignoring other symbols. Order of sorted strings is lexicographical.
* CmpLettersInvZA - compare strings from right to left only by letters (register independent), ignoring other symbols. Order of sorted strings is reversed lexicographical.
* StrCmpAZ - compare strings from left to right, letters will be processed register independent. Order of sorted strings is lexicographical.
* StrCmpZA - compare strings from left to right, letters will be processed register independent. Order of sorted strings is reversed lexicographical.
* StrCmpInvAZ - compare strings from right to left, letters will be processed register independent. Order of sorted strings is lexicographical.
* StrCmpInvZA - compare strings from right to left, letters will be processed register independent. Order of sorted strings is reversed lexicographical.


## Contacts

If you will have any questions, suggestions and so on, just text me)

Mail: 55555artem.kulakov@gmail.com
