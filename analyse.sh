#!/bin/bash

tail -n +16 p.out | build/analyse $@
