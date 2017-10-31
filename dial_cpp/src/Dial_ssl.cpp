#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/crypto.h>

#include "Dial_common.h"
#include "Dial_ssl.h"



/**
 * Connect to a host using an encrypted stream
 */
BIO* connect_encrypted(char* host_and_port, char* store_path, SSL_CTX** ctx, SSL** ssl) {

    BIO* bio = NULL;
    int r = 0;

    /* Set up the SSL pointers */
    *ctx = SSL_CTX_new(TLSv1_client_method());
	if(NULL == *ctx) {
        debug_printf(LOG_LEVEL_ERROR,"connect_encrypted:SSL_CTX_new failed!\n", store_path);
        return NULL;
    }
	
    *ssl = NULL;
	
    r = SSL_CTX_load_verify_locations(*ctx, store_path, NULL);
    if (r == 0) {
        debug_printf(LOG_LEVEL_ERROR,"connect_encrypted:Unable to load the trust store from %s.\n", store_path);
		SSL_CTX_free(*ctx);
		return NULL;
    }

    /* Setting up the BIO SSL object */
    bio = BIO_new_ssl_connect(*ctx);
    BIO_get_ssl(bio, ssl);
    if (!(*ssl)) {
        debug_printf(LOG_LEVEL_ERROR,"connect_encrypted:Unable to allocate SSL pointer.\n");
		BIO_free_all(bio);
		SSL_CTX_free(*ctx);	
        return NULL;
    }
    SSL_set_mode(*ssl, SSL_MODE_AUTO_RETRY);

    /* Attempt to connect */
    BIO_set_conn_hostname(bio, host_and_port);

    /* Verify the connection opened and perform the handshake */
    if (BIO_do_connect(bio) < 1) {
        debug_printf(LOG_LEVEL_ERROR,"connect_encrypted:Unable to connect BIO.%s\n", host_and_port);
		BIO_free_all(bio);
		SSL_CTX_free(*ctx);	
		return NULL;
    }

    if (SSL_get_verify_result(*ssl) != X509_V_OK) {

        debug_printf(LOG_LEVEL_ERROR,"connect_encrypted:Unable to verify connection result.\n");
    }

    return bio;
}

/**
 * Read a from a stream and handle restarts if nessecary
 */
ssize_t read_from_stream(BIO* bio, char* buffer, ssize_t length) {

    ssize_t r = -1;
	long size = 0;

    do{
        r = BIO_read(bio, buffer+size, 1024);
        if (r == 0) {

            debug_printf(LOG_LEVEL_ERROR,"Reached the end of the data stream.\n");
            //continue;

        } else if (r < 0) {

            if (!BIO_should_retry(bio)) {

                debug_printf(LOG_LEVEL_ERROR,"BIO_read should retry test failed.\n");
               // continue;
            }

            /* It would be prudent to check the reason for the retry and handle
             * it appropriately here */
        }
		size += r;
	}while (r > 0 && size <= length);

    return r;
}

/**
 * Write to a stream and handle restarts if nessecary
 */
int write_to_stream(BIO* bio, char* buffer, ssize_t length) {

    ssize_t r = -1;

    while (r < 0) {

        r = BIO_write(bio, buffer, length);
        if (r <= 0) {

            if (!BIO_should_retry(bio)) {

                debug_printf(LOG_LEVEL_ERROR,"BIO_read should retry test failed.\n");
                continue;
            }

            /* It would be prudent to check the reason for the retry and handle
             * it appropriately here */
        }

    }

    return r;
}



void 
init_openssl() 
{

    /* call the standard SSL init functions */
    SSL_load_error_strings();
    SSL_library_init();
    
    ERR_load_BIO_strings();
    OpenSSL_add_all_algorithms();

    /* seed the random number system - only really nessecary for systems without '/dev/random' */
}

int
https_get_status_code(char *sendbuf,
								int buflen,
								char *cert_file,
								char *ip,
								int port)
{
    char host_and_port[1024] = {0}; 
    //char* store_path = "mycert.pem"; 
    char buffer[1024]={0};
	int status_code = 0; 

    BIO* bio;
    SSL_CTX* ctx = NULL;
    SSL* ssl = NULL;

	if(NULL == sendbuf || buflen < 1 || NULL == ip || port < 0) {
		debug_printf(LOG_LEVEL_ERROR,"https_get_status_code:invalid parameters!!\n");
		return -1;
	}
		

	sprintf(host_and_port,"%s:%d",ip,port);
	debug_printf(LOG_LEVEL_DEBUG,"https_get_status_code:host_and_port=%s\n",host_and_port);
	
    /* initilise the OpenSSL library */
    //init_openssl();

	if ((bio = connect_encrypted(host_and_port, cert_file, &ctx, &ssl)) == NULL)
		return -1;


	write_to_stream(bio, sendbuf, buflen);
	read_from_stream(bio, buffer, sizeof(buffer));
	status_code = atoi(buffer + 9);
	
	//printf("%s\r\n", buffer);

	 /* clean up the SSL context resources for the encrypted link */
	BIO_free_all(bio);
	SSL_CTX_free(ctx);
	ERR_free_strings();

	return status_code;
}

#if 0
{
	SSL_CTX *ctx;
	SSL *ssl;
	int bytes = 0;
	char buf[1024] = {0};
	int status_code = -1;
	int ret = 0;
	int err = 0;
	debug_printf(LOG_LEVEL_DEBUG,"https_get_status_code:buflen=%d,sendbuf=%s\n",buflen,sendbuf);

	
	SSL_library_init();
	
	ctx = InitCTX();
	if(NULL == ctx) {
		debug_printf(LOG_LEVEL_ERROR,"https_get_status_code:InitCTX failed\n");
		return -1;
	}
	
	ssl = SSL_new(ctx);     /* create new SSL connection state */
	ret = SSL_set_fd(ssl, fd);   /* attach the socket descriptor */
	if(ret == 0) {
		debug_printf(LOG_LEVEL_ERROR,"https_get_status_code:SSL link socket failed!\n");
		return -1;
	}
	
	if(SSL_connect(ssl) == -1)	/* perform the connection */
	{
		debug_printf(LOG_LEVEL_ERROR,"https_get_status_code:SSL_connect failed\n");
	}
	else
	{ 
		debug_printf(LOG_LEVEL_DEBUG,"https_get_status_code:Connected with <%s> encryption\n", SSL_get_cipher(ssl));

		ShowCerts(ssl); 	  /* get any certs */

		ret = SSL_write(ssl, sendbuf,buflen);  /* encrypt & send message */
		err = SSL_get_error(ssl,ret);
		debug_printf(LOG_LEVEL_DEBUG,"https_get_status_code:SSL_write err=%d\n",err);
		usleep(1000);
		bytes = SSL_read(ssl,buf,sizeof(buf));/* get reply & decrypt */
		err = SSL_get_error(ssl,bytes);
		debug_printf(LOG_LEVEL_DEBUG,"https_get_status_code:SSL_write err=%d\n",err);	
		debug_printf(LOG_LEVEL_DEBUG,"https_get_status_code:read_bytes=%d,write_bytes=%d,Received->%s\n",ret,bytes,buf);

		status_code = atoi(buf + 9);
	}
	
	//close(fd);		  /* close socket */
	ret = SSL_shutdown(ssl);
    if (ret != 1)
    {
        debug_printf(LOG_LEVEL_DEBUG,"SSL shutdown failed!\n");
        return -1;
    }
	
	SSL_free(ssl);		 /* release connection state */
	ERR_free_strings();
	SSL_CTX_free(ctx);		 /* release context */

	return status_code;
}

#endif


int
https_get_check_result(char *rcvbuf,
								char *sendbuf,
								int snd_buflen,
								int rcv_buflen,
								char *cert_file,
								char *ip,
								int port)
{
    char host_and_port[1024] = {0}; 
    //char* store_path = "mycert.pem"; 
    //char buffer[1024]={0};
	int status_code = 0; 

    BIO* bio = NULL;
    SSL_CTX* ctx = NULL;
    SSL* ssl = NULL;

	if(NULL == sendbuf || snd_buflen < 1 || NULL == ip || port < 0) {
		debug_printf(LOG_LEVEL_ERROR,"https_get_check_result:invalid parameters!!\n");
		return -1;
	}
		

	sprintf(host_and_port,"%s:%d",ip,port);
	debug_printf(LOG_LEVEL_DEBUG,"https_get_check_result:host_and_port=%s\n",host_and_port);
	
    /* initilise the OpenSSL library */
    //init_openssl();

	if ((bio = connect_encrypted(host_and_port, cert_file, &ctx, &ssl)) == NULL)
		return -1;


	write_to_stream(bio, sendbuf, snd_buflen);
	read_from_stream(bio, rcvbuf, rcv_buflen);
	status_code = atoi(rcvbuf + 9);
	
	//printf("%s\r\n", buffer);

	 /* clean up the SSL context resources for the encrypted link */
	BIO_free_all(bio);
	SSL_CTX_free(ctx);
	ERR_free_strings();


	return status_code;
}

