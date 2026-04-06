/* The example of esp-idf
 *
 * This sample code is in the public domain.
 */

#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "mbedtls/platform.h"
#include "mbedtls/net_sockets.h"
#include "mbedtls/esp_debug.h"
#include "mbedtls/ssl.h"
#if ESP_IDF_VERSION_MAJOR == 5
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#endif
#include "mbedtls/error.h"
#ifdef CONFIG_MBEDTLS_SSL_PROTO_TLS1_3
#include "psa/crypto.h"
#endif
#include "esp_crt_bundle.h"

extern const uint8_t server_crt_start[] asm("_binary_server_crt_start");
extern const uint8_t server_crt_end[]   asm("_binary_server_crt_end");

/* Constants that aren't configurable in menuconfig */
#define SERVER_HOST CONFIG_SERVER_HOST
#define SERVER_PORT CONFIG_SERVER_PORT
#define WEB_URL "https://www.howsmyssl.com/a/check"

static const char *TAG = "SSL";

static const char *REQUEST = "GET " WEB_URL " HTTP/1.0\r\n"
	"Host: "SERVER_HOST"\r\n"
	"User-Agent: esp-idf/1.0 esp32\r\n"
	"\r\n";


void ssl_tcp(void *pvParameters)
{
	char buf[512];
	int ret, flags, len;

	ESP_LOGI(pcTaskGetName(NULL), "Start");
#if ESP_IDF_VERSION_MAJOR == 5
	mbedtls_entropy_context entropy;
	mbedtls_ctr_drbg_context ctr_drbg;
#endif
	mbedtls_ssl_context ssl;
	mbedtls_x509_crt cacert;
	mbedtls_ssl_config conf;
	mbedtls_net_context server_fd;

#ifdef CONFIG_MBEDTLS_SSL_PROTO_TLS1_3
	psa_status_t status = psa_crypto_init();
	if (status != PSA_SUCCESS) {
		ESP_LOGE(pcTaskGetName(NULL), "Failed to initialize PSA crypto, returned %d", (int) status);
		return;
	}
#endif

	mbedtls_ssl_init(&ssl);
	mbedtls_x509_crt_init(&cacert);
#if ESP_IDF_VERSION_MAJOR == 5
	mbedtls_ctr_drbg_init(&ctr_drbg);
	ESP_LOGI(pcTaskGetName(NULL), "Seeding the random number generator");
#endif

	mbedtls_ssl_config_init(&conf);

#if ESP_IDF_VERSION_MAJOR == 5
	mbedtls_entropy_init(&entropy);
	if((ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy,
									NULL, 0)) != 0)
	{
		ESP_LOGE(pcTaskGetName(NULL), "mbedtls_ctr_drbg_seed returned %d", ret);
		abort();
	}
#endif

	ESP_LOGI(pcTaskGetName(NULL), "Loading the CA root certificate...");

	ret = mbedtls_x509_crt_parse(&cacert, server_crt_start,
								 server_crt_end - server_crt_start);
	if(ret < 0)
	{
		ESP_LOGE(pcTaskGetName(NULL), "mbedtls_x509_crt_parse returned -0x%x", -ret);
		abort();
	}


	ESP_LOGI(pcTaskGetName(NULL), "Setting hostname for TLS session...");

	 /* Hostname set here should match CN in server certificate */
	if((ret = mbedtls_ssl_set_hostname(&ssl, SERVER_HOST)) != 0)
	{
		ESP_LOGE(pcTaskGetName(NULL), "mbedtls_ssl_set_hostname returned -0x%x", -ret);
		abort();
	}

	ESP_LOGI(pcTaskGetName(NULL), "Setting up the SSL/TLS structure...");

	if((ret = mbedtls_ssl_config_defaults(&conf,
										  MBEDTLS_SSL_IS_CLIENT,
										  MBEDTLS_SSL_TRANSPORT_STREAM,
										  MBEDTLS_SSL_PRESET_DEFAULT)) != 0)
	{
		ESP_LOGE(pcTaskGetName(NULL), "mbedtls_ssl_config_defaults returned %d", ret);
		goto exit;
	}

	mbedtls_ssl_conf_authmode(&conf, MBEDTLS_SSL_VERIFY_REQUIRED);
	mbedtls_ssl_conf_ca_chain(&conf, &cacert, NULL);
#if ESP_IDF_VERSION_MAJOR == 5
	mbedtls_ssl_conf_rng(&conf, mbedtls_ctr_drbg_random, &ctr_drbg);
#endif
#ifdef CONFIG_MBEDTLS_DEBUG
	mbedtls_esp_enable_debug_log(&conf, CONFIG_MBEDTLS_DEBUG_LEVEL);
#endif

	if ((ret = mbedtls_ssl_setup(&ssl, &conf)) != 0)
	{
		ESP_LOGE(pcTaskGetName(NULL), "mbedtls_ssl_setup returned -0x%x", -ret);
		goto exit;
	}

	mbedtls_net_init(&server_fd);

	char server_port[16];
	snprintf(server_port, sizeof(server_port), "%d", SERVER_PORT);

	ESP_LOGI(pcTaskGetName(NULL), "Connecting to %s:%s...", SERVER_HOST, server_port);

	if ((ret = mbedtls_net_connect(&server_fd, SERVER_HOST, server_port, MBEDTLS_NET_PROTO_TCP)) != 0)
	{
		ESP_LOGE(pcTaskGetName(NULL), "mbedtls_net_connect returned -%x", -ret);
		goto exit;
	}

	ESP_LOGI(pcTaskGetName(NULL), "Connected.");

	mbedtls_ssl_set_bio(&ssl, &server_fd, mbedtls_net_send, mbedtls_net_recv, NULL);

	ESP_LOGI(pcTaskGetName(NULL), "Performing the SSL/TLS handshake...");

	while ((ret = mbedtls_ssl_handshake(&ssl)) != 0)
	{
		if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE)
		{
			ESP_LOGE(pcTaskGetName(NULL), "mbedtls_ssl_handshake returned -0x%x", -ret);
			goto exit;
		}
	}

	ESP_LOGI(pcTaskGetName(NULL), "Verifying peer X.509 certificate...");

	if ((flags = mbedtls_ssl_get_verify_result(&ssl)) != 0)
	{
		/* In real life, we probably want to close connection if ret != 0 */
		ESP_LOGW(pcTaskGetName(NULL), "Failed to verify peer certificate!");
		bzero(buf, sizeof(buf));
		mbedtls_x509_crt_verify_info(buf, sizeof(buf), "  ! ", flags);
		ESP_LOGW(pcTaskGetName(NULL), "verification info: %s", buf);
	}
	else {
		ESP_LOGI(pcTaskGetName(NULL), "Certificate verified.");
	}

	ESP_LOGI(pcTaskGetName(NULL), "Cipher suite is %s", mbedtls_ssl_get_ciphersuite(&ssl));

	ESP_LOGI(pcTaskGetName(NULL), "Writing...");

	size_t written_bytes = 0;
	do {
		ret = mbedtls_ssl_write(&ssl, (const unsigned char *)REQUEST + written_bytes,
			strlen(REQUEST) - written_bytes);
		if (ret >= 0) {
			ESP_LOGI(pcTaskGetName(NULL), "%d bytes written", ret);
			written_bytes += ret;
		} else if (ret != MBEDTLS_ERR_SSL_WANT_WRITE && ret != MBEDTLS_ERR_SSL_WANT_READ) {
			ESP_LOGE(pcTaskGetName(NULL), "mbedtls_ssl_write returned -0x%x", -ret);
			goto exit;
		}
	} while(written_bytes < strlen(REQUEST));

	ESP_LOGI(pcTaskGetName(NULL), "Reading...");

	do
	{
		len = sizeof(buf) - 1;
		bzero(buf, sizeof(buf));
		ret = mbedtls_ssl_read(&ssl, (unsigned char *)buf, len);

#if CONFIG_MBEDTLS_SSL_PROTO_TLS1_3 && CONFIG_MBEDTLS_CLIENT_SSL_SESSION_TICKETS
		if (ret == MBEDTLS_ERR_SSL_RECEIVED_NEW_SESSION_TICKET) {
			ESP_LOGD(pcTaskGetName(NULL), "got session ticket in TLS 1.3 connection, retry read");
			continue;
		}
#endif // CONFIG_MBEDTLS_SSL_PROTO_TLS1_3 && CONFIG_MBEDTLS_CLIENT_SSL_SESSION_TICKETS

		if (ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE) {
			continue;
		}

		if (ret == MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY) {
			ESP_LOGI(pcTaskGetName(NULL), "MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY");
			ret = 0;
			break;
		}

		if (ret < 0) {
			ESP_LOGE(pcTaskGetName(NULL), "mbedtls_ssl_read returned -0x%x", -ret);
			break;
		}

		if (ret == 0) {
			ESP_LOGI(pcTaskGetName(NULL), "connection closed");
			break;
		}

		len = ret;
		ESP_LOGD(pcTaskGetName(NULL), "%d bytes read", len);
		/* Print response directly to stdout as it is read */
		for (int i = 0; i < len; i++) {
			putchar(buf[i]);
		}
	} while(1);

	mbedtls_ssl_close_notify(&ssl);

exit:
	mbedtls_ssl_session_reset(&ssl);
	mbedtls_net_free(&server_fd);

	if (ret != 0) {
		mbedtls_strerror(ret, buf, 100);
		ESP_LOGE(pcTaskGetName(NULL), "Last error was: -0x%x - %s", -ret, buf);
	}
	vTaskDelete(NULL);
}
