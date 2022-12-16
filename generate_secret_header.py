#! /usr/bin/env python

import os, json
import subprocess

try:
    secrets = subprocess.check_output("doppler secrets download --no-file".split(" "))
except subprocess.CalledProcessError:
    print("error running doppler. Has it been initialized properly? Check readme.")
    exit()

try:
    secrets_json = json.loads(secrets)
except json.decoder.JSONDecodeError:
    print("error decoding doppler secrets. The gotten data is:")
    print(secrets)
    exit()

with open("include/secrets.h", 'w', encoding = 'utf-8') as f:
    for k,v in secrets_json.items():
        f.write(f'#define {k} "{v}"\n')

print("Header succesfully generated.")