#!/bin/bash

valgrind --leak-check=full ./ghostprotocold 2>&1 | tee valgrind.log
