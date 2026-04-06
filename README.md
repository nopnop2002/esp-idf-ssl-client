# esp-idf-ssl-client
Simple ssl client for esp-idf.   

ESP-IDF includes [this](https://github.com/Mbed-TLS/mbedtls) mbedtsl library.   
You can use this components as standard.   
ESP-IDF includes many example codes, but there wasn't a simple SSL communication example, so I created one.   
You can use mbedtls_ssl_write/mbedtls_ssl_read instead of read/write.   

# Software requirements
ESP-IDF V5.0 or later.   
ESP-IDF V4.4 release branch reached EOL in July 2024.   

# Installation
```
git clone https://github.com/nopnop2002/esp-idf-ssl-client
cd esp-idf-ssl-client/
chmod 777 mkkey.sh
./mkkey.sh
idf.py menuconfig
idf.py flash
```

mkkey.sh creates a server certificate file.   
The server certificate file is associated with the SSL server's IP address.   
mkkey.sh automatically obtains the SSL server's IP address.   
To manually configure the SSL server's IP address, change it as follows:   
```
IP="192.168.0.123"
openssl req -x509 -new -nodes -key server.key -subj "/CN=${IP}" -days 10000 -out server.crt
```


# Configuration   
![Image](https://github.com/user-attachments/assets/9fa2ecf4-d36e-4b74-9c81-038cb5c8da5a)
![Image](https://github.com/user-attachments/assets/23fb100a-7ab4-4790-bb25-0f7a5d6f19d5)


# Start ssl server
```
cd esp-idf-ssl-client/
cd clang-tls-communication
make
./server
```

![Image](https://github.com/user-attachments/assets/e030e803-c799-4e28-91b6-c2ab1ed66a47)
