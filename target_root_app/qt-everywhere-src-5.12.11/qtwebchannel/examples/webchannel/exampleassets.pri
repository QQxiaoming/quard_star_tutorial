# This adds the qwebchannel js library to an example, creating a self-contained bundle
jslib = $$PWD/shared/qwebchannel.js

# This installs all assets including qwebchannel.js, regardless of the source.
exampleassets.files += $$jslib
INSTALLS += exampleassets

# This code ensures that all assets are present in the build directory.

!equals(_PRO_FILE_PWD_, $$OUT_PWD) {
    # Shadow build, copy all example assets.
    assetcopy.files += $$exampleassets.files
} else {
    # Just copy jslib - other assets are already in place.
    assetcopy.files = $$jslib
}

assetcopy.path = $$OUT_PWD
COPIES += assetcopy
