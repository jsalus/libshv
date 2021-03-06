TEMPLATE = subdirs
CONFIG += ordered

SUBDIRS += \
    3rdparty \
    libshvchainpack \
    libshvcore \
    libshvcoreqt \
    libshviotqt \
    libshvbroker \
    utils \
    samples \

qtHaveModule(gui) {
SUBDIRS += \
    libshvvisu \
}

CONFIG(debug, debug|release) {
SUBDIRS += \
    tests \
}
