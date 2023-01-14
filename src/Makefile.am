bin_PROGRAMS = gridcoin_address_estimator

SUBDIRS = Gridcoin-Research/src/

LIBGRIDCOIN_UTIL = Gridcoin-Research/src/libgridcoin_util.a
# Hardcoded until I get around cleaning the src/Makefile.am of Gridcoin.
BDB_LIBS = -ldb_cxx
BOOST_LIBS = -L/usr/lib64 -lboost_system -lboost_filesystem -lboost_iostreams -lboost_thread -lpthread
CRYPTO_LIBS = -lcrypto
CURL_LIBS = -lcurl
LIBZIP_LIBS = -lzip
MINIUPNPC_LIBS = -lminiupnpc
SSL_LIBS = -lssl
LIBGRIDCOIN_CRYPTO = Gridcoin-Research/src/crypto/libgridcoin_crypto_base.a Gridcoin-Research/src/crypto/libgridcoin_crypto_sse41.a Gridcoin-Research/src/crypto/libgridcoin_crypto_shani.a Gridcoin-Research/src/crypto/libgridcoin_crypto_avx2.a
LIBUNIVALUE = Gridcoin-Research/src/univalue/libunivalue.la
LIBSECP256K1 = Gridcoin-Research/src/secp256k1/libsecp256k1.la
LIBLEVELDB = Gridcoin-Research/src/leveldb/libleveldb.a Gridcoin-Research/src/crc32c/libcrc32c.a Gridcoin-Research/src/crc32c/libcrc32c_sse42.a

gridcoin_address_estimator_CXXFLAGS = -IGridcoin-Research/src/ -IGridcoin-Research/src/leveldb/include
gridcoin_address_estimator_LDADD = $(LIBGRIDCOIN_UTIL) $(BOOST_LIBS) $(LIBGRIDCOIN_CRYPTO) $(LIBUNIVALUE) $(CRYPTO_LIBS) $(LIBSECP256K1) $(LIBLEVELDB) $(LIBZIP_LIBS) $(CURL_LIBS) $(SSL_LIBS) $(BDB_LIBS) $(MINIUPNPC_LIBS)
gridcoin_address_estimator_SOURCES = main.cpp
