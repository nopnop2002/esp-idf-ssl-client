# esp-idf-ssl-client
Simple ssl client for esp-idf.   

ESP-IDF includes [this](https://github.com/Mbed-TLS/mbedtls) mbedtsl library.   
You can use this components as standard.   
ESP-IDF includes many example codes, but there wasn't a simple SSL communication example, so I created one.   

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

![config-main](https://user-images.githubusercontent.com/6020549/120054821-3d755500-c06d-11eb-950c-d357d0a9fdef.jpg)
![config-app](https://user-images.githubusercontent.com/6020549/166416531-7fa74d94-86fc-4cac-a568-74de07d7a051.jpg)

# Start ssl server
```
cd esp-idf-ssl-client/
cd clang-tls-communication
make
./server
```

