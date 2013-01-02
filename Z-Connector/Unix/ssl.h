#ifndef _SSL_H_
#define _SSL_H_

#define DEFAULT_HANDLER(s, ssl, r) ERR(s ": (%d) %s\n", ERR_get_error(), ERR_reason_error_string(ERR_get_error()))

// initialize OpenSSL environment
int SSLInit();

// create a reusable SSL context
SSL_CTX *SSLCreateContext(const char *certFile, const char *keyFile, const char *caCertFile);

// free SSL context
void SSLFreeContext(SSL_CTX *ctx);

// perform handshake
SSL *SSLHandshake(SOCKET sock, SSL_CTX *context);

// close SSL session
void SSLCloseSession(SSL *ssl);

#endif