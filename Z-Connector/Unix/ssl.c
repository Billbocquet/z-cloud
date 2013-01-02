#include "common.h"
#include "log.h"

#include <openssl/x509.h>
#include <openssl/err.h>
#include "ssl.h"

#define SSLERROR ERR_reason_error_string(ERR_get_error())

#define RETURN_CTX(x) \
	SSLFreeContext(ctx); \
	return (x)

int SSLInit()
{
	DBG("Initialising OpenSSL environment...\n");

	SSL_library_init();
	SSL_load_error_strings();

	return 0;
}

SSL_CTX *SSLCreateContext(const char *certFile, const char *keyFile, const char *caCertFile)
{
	SSL_CTX *ctx = NULL;
	struct stack_st_X509_NAME *ca = NULL;

	DBG("Allocating SSL context...\n");
	ctx = SSL_CTX_new(TLSv1_client_method());
	if (ctx == NULL)
	{
		ERR("Failed to allocate SSL context: %s\n", SSLERROR);
		RETURN_CTX(NULL);
	}
	
	DBG("Loading personal certificate...\n");
	if (1 != SSL_CTX_use_certificate_chain_file(ctx, certFile))
	{
		ERR("Failed to load personal certificate: %s\n", SSLERROR);
		RETURN_CTX(NULL);
	}

	DBG("Loading personal certificate key...\n");
	if (1 != SSL_CTX_use_PrivateKey_file(ctx, keyFile, SSL_FILETYPE_PEM))
	{
		ERR("Failed to load personal certificate key: %s\n", SSLERROR);
		RETURN_CTX(NULL);
	}
	
	DBG("Loading CA certificate...\n");
	if (1 != SSL_CTX_load_verify_locations(ctx, caCertFile, NULL))
	{
		ERR("Failed to load CA certificate: %s\n", SSLERROR);
		RETURN_CTX(NULL);
	}
		
	DBG("Setting verification mode...\n");
	SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, NULL);
	
	return ctx;
}

void SSLFreeContext(SSL_CTX *ctx)
{
	if (ctx == NULL) return;

	DBG("Freeing SSL context...\n"); 
	SSL_CTX_free(ctx);
}

SSL *SSLHandshake(SOCKET sock, SSL_CTX *context)
{
	SSL *ssl = NULL;
	BIO *bio;
	int r, i;
	
	INFO("Establishing SSL connection...\n");

	DBG("Allocating SSL session...\n");
	ssl = SSL_new(context);
	if (ssl == NULL)
	{
		ERR("Failed to allocate SSL session: %s\n", SSLERROR);

		return NULL;
	}
	
	DBG("Assigning socket...\n");
	bio = BIO_new_socket(sock, BIO_NOCLOSE);
	SSL_set_bio(ssl, bio, bio);

	INFO("Performing SSL handshake...\n");
	
	DBG("Initialising SSL state...\n");
	SSL_set_connect_state(ssl);

	DBG("Sending handshake...\n");
	if (1 != SSL_do_handshake(ssl))
	{
		ERR("Failed to perform handshake: %s\n", SSLERROR);

		DBG("Freeing SSL session...\n");
		SSL_free(ssl);

		return NULL;
	}

	return ssl;
}

void SSLCloseSession(SSL *ssl)
{
	if (ssl == NULL) return;

	DBG("Shutting down SSL session...\n");
	SSL_shutdown(ssl);

	DBG("Freeing SSL session...\n"); 
	SSL_free(ssl);
}