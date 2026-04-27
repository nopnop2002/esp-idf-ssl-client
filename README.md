# esp-idf-ssl-client
Simple ssl client for esp-idf.   

ESP-IDF includes [this](https://github.com/Mbed-TLS/mbedtls) mbedtsl library.   
You can use this components as standard.   
ESP-IDF includes many example codes, but there wasn't a simple SSL communication example, so I created one.   
You can use mbedtls_ssl_write/mbedtls_ssl_read instead of read/write.   

# Software requirements
ESP-IDF V5.0 or later.   
ESP-IDF V4.4 release branch reached EOL in July 2024.   

__Notes on mbedtls version__   
ESP-IDF V5.x includes mbedtls Version 3.x.x.   
ESP-IDF V6.x includes mbedtls Version 4.x.x.   
In mbedtls V3, applications using TLS needed to provide a random generator, generally by instantiating an entropy context (mbedtls_entropy_context) and a DRBG context (mbedtls_ctr_drbg_context or mbedtls_hmac_drbg_context).    
In mbedtls V4, all features that require a random number generator (RNG) now use the random number generator provided by the PSA subsystem.   
So It is no longer necessary to instantiate the entropy context and the DRBG context.   

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
mkkey.sh automatically retrieves the IP address of the server on which the script is executed and treats that address as an SSL server.   
To manually configure the SSL server's IP address, modify the script as follows:   
```
IP="192.168.0.123"
openssl req -x509 -new -nodes -key server.key -subj "/CN=${IP}" -days 10000 -out server.crt
```


# Configuration   
![Image](https://github.com/user-attachments/assets/9fa2ecf4-d36e-4b74-9c81-038cb5c8da5a)
![Image](https://github.com/user-attachments/assets/23fb100a-7ab4-4790-bb25-0f7a5d6f19d5)


# Start the SSL server
- C language
	```
	cd esp-idf-ssl-client/
	cd clang-tls-communication
	make
	./server
	```
	![Image](https://github.com/user-attachments/assets/e030e803-c799-4e28-91b6-c2ab1ed66a47)

- python script
	```
	cd esp-idf-ssl-client/
	cd python-tls-communication
	python3 server.py
	```
	![Image](https://github.com/user-attachments/assets/a9a0bfec-fc3f-48c8-9fd3-f27596dfa961)
