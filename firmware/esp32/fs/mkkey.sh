#!/bin/sh
openssl req -newkey rsa:2048 -nodes -keyout secret/key.pem -x509 -days 3650 -out secret/cert.pem -subj "/CN=esp32leddriver"
