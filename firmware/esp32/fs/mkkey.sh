#!/bin/sh
ca_key=secret/ca_key.pem
ca_cert=secret/ca_cert.pem
csr=secret/csr.pem
key=secret/key.pem
cert=secret/cert.pem

openssl ecparam -genkey -name secp256r1 -out "$ca_key"
openssl req -x509 -new -SHA256 -nodes -key "$ca_key" -days 3650 -out "$ca_cert" -subj "/CN=esp32leddriver"
openssl ecparam -genkey -name secp256r1 -out "$key"
openssl req -new -SHA256 -key "$key" -nodes -out "$csr" -subj "/CN=esp32leddriver"
openssl x509 -req -SHA256 -days 3650 -in "$csr" -CA "$ca_cert" -CAkey "$ca_key" -CAcreateserial -out "$cert"
