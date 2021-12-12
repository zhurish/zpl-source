#############################################################################
# DEFINE
#ZPL_LIBSSH_ZLIB=true
#ZPL_LIBSSH_GCRYPT=false
#ZPL_LIBSSH_MBEDTLS=false
#ZPL_LIBSSH_CRYPTO=true
#ZPL_LIBSSH_OPENSSL_ED25519=true
#ZPL_LIBSSH_NACL=false
#ZPL_LIBSSH_SFTP=true
#ZPL_LIBSSH_SERVER=true
#ZPL_LIBSSH_GSSAPI=false
#ZPL_LIBSSH_GEX=false
#ZPL_LIBSSH_PCAP=true
#ZPL_LIBSSH_BLOWFISH=false
#ZPL_LIBSSH_PTHREAD=true




###########################################################################
MODULEDIR = component/ssh
#ZPLINCLUDE += -I$(OPENSSH_ROOT)/include
#OS
LIBSSHOBJS += agent.o \
  auth.o \
  base64.o \
  bignum.o \
  ssh_buffer.o \
  callbacks.o \
  channels.o \
  client.o \
  config.o \
  connect.o \
  connector.o \
  curve25519.o \
  dh.o \
  ecdh.o \
  error.o \
  getpass.o \
  init.o \
  kdf.o \
  kex.o \
  known_hosts.o \
  knownhosts.o \
  legacy.o \
  log.o \
  match.o \
  messages.o \
  misc.o \
  options.o \
  packet.o \
  packet_cb.o \
  packet_crypt.o \
  pcap.o \
  pki.o \
  pki_container_openssh.o \
  poll.o \
  session.o \
  scp.o \
  socket.o \
  string.o \
  threads.o \
  wrapper.o \
  bcrypt_pbkdf.o \
  blowfish.o \
  chacha.o \
  poly1305.o \
  chachapoly.o \
  config_parser.o \
  token.o \
  pki_ed25519_common.o \

ifeq ($(strip $(ZPL_LIBSSH_GCRYPT)),true)
LIBSSHOBJS += \
        pthread_libgcrypt.o \
        libgcrypt.o \
        gcrypt_missing.o \
        pki_gcrypt.o \
        ecdh_gcrypt.o \
        dh_key.o \
        pki_ed25519.o \
        ed25519.o \
        fe25519.o \
        ge25519.o \
        sc25519.o 

SSH_PLDEFINE +=-DHAVE_LIBGCRYPT
endif
ifeq ($(strip $(ZPL_LIBSSH_MBEDTLS)),true)
LIBSSHOBJS += \
        mbedtls.o \
        libmbedcrypto.o \
        mbedcrypto_missing.o \
        pki_mbedcrypto.o \
        ecdh_mbedcrypto.o \
        dh_key.o \
        pki_ed25519.o \
        ed25519.o \
        fe25519.o \
        ge25519.o \
        sc25519.o 
SSH_PLDEFINE +=-DHAVE_LIBMBEDCRYPTO 
endif 
ifeq ($(strip $(ZPL_LIBSSH_CRYPTO)),true)
LIBSSHOBJS += \
        pthread_libcrypto.o \
        pki_crypto.o \
        mbedcrypto_missing.o \
        libcrypto.o \
        ecdh_crypto.o \
        dh_crypto.o 
LIBSSHOBJS += libcrypto-compat.o
SSH_PLDEFINE +=-DHAVE_LIBCRYPTO=1
endif

ifeq ($(strip $(ZPL_LIBSSH_OPENSSL_ED25519)),false)
LIBSSHOBJS += pki_ed25519.o \
        ed25519.o \
        fe25519.o \
        ge25519.o \
        sc25519.o 
endif

ifeq ($(strip $(ZPL_LIBSSH_SFTP)),true)
LIBSSHOBJS += sftp.o
SSH_PLDEFINE +=-DWITH_SFTP=1
ifeq ($(strip $(ZPL_LIBSSH_SERVER)),true)
LIBSSHOBJS += sftpserver.o
endif
endif

ifeq ($(strip $(ZPL_LIBSSH_SERVER)),true)
LIBSSHOBJS += server.o 
LIBSSHOBJS += bind_config.o 
LIBSSHOBJS += bind.o 
SSH_PLDEFINE +=-DWITH_SERVER=1
endif

ifeq ($(strip $(ZPL_LIBSSH_GEX)),true)
LIBSSHOBJS +=   dh-gex.o
SSH_PLDEFINE +=-DWITH_GEX=1
endif


ifeq ($(strip $(ZPL_LIBSSH_ZLIB)),true)
LIBSSHOBJS += gzip.o
SSH_PLDEFINE +=-DWITH_ZLIB=1
endif

ifeq ($(strip $(ZPL_LIBSSH_GSSAPI)),true)
#if (ZPL_LIBSSH_GSSAPI AND GSSAPI_FOUND)
LIBSSHOBJS += gssapi.o
#endif (ZPL_LIBSSH_GSSAPI AND GSSAPI_FOUND)
SSH_PLDEFINE +=-DWITH_GSSAPI=1
endif

ifeq ($(strip $(ZPL_LIBSSH_NACL)),false)
#if (NOT ZPL_LIBSSH_NACL)
LIBSSHOBJS += curve25519_ref.o
#endif (NOT ZPL_LIBSSH_NACL)
endif
ifeq ($(strip $(ZPL_LIBSSH_PTHREAD)),true)
LIBSSHOBJS += pthread.o  noop.o
SSH_PLDEFINE +=-DHAVE_PTHREAD=1
endif 
ifeq ($(strip $(ZPL_LIBSSH_BLOWFISH)),true)
SSH_PLDEFINE +=-DWITH_BLOWFISH_CIPHER=1
endif 
ifeq ($(strip $(ZPL_LIBSSH_PCAP)),true)
SSH_PLDEFINE +=-DWITH_PCAP=1
endif 


OBJS += ssh_util.o
OBJS += ssh_connect.o
OBJS += ssh_keymgt.o
OBJS += ssh_authentication.o
OBJS += ssh_client.o
OBJS += ssh_sftp.o
OBJS += ssh_sftpd.o
OBJS += ssh_scp.o
OBJS += sshd_main.o
OBJS += ssh_api.o
ifeq ($(strip $(ZPL_SHELL_MODULE)),true)
OBJS += cmd_ssh.o
endif
#############################################################################
# LIB
###########################################################################
LIBS = libssh.a
