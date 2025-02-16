bin_PROGRAMS = gridcoin_address_estimator

SUBDIRS = Gridcoin-Research/src/

gridcoin_address_estimator_CXXFLAGS = -IGridcoin-Research/src/ -IGridcoin-Research/src/leveldb/include
gridcoin_address_estimator_LDADD = $(GRIDCOINRESEARCHD_LDFLAGS)
gridcoin_address_estimator_SOURCES = main.cpp
