#!/bin/bash

CYAN='\e[36m'
PURPLE='\e[35m'
NC='\e[0m' # No Color

if [[ $EUID -ne 0 ]]; then
   echo -e "${PURPLE}Permission denied${NC}" 1>&2
   exit 100
fi

if [ -e install_manifest.txt ]
then
    echo -e "${CYAN}Uninstall the project...${NC}"
    xargs rm -v < install_manifest.txt
    rm -f install_manifest.txt
fi
