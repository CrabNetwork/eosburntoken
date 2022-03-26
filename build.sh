#!/bin/bash

eosio-cpp -abigen -I include -R resource -contract token -o token.wasm src/token.cpp
