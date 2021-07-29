/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Assistant of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <process.h>
#else
#include <unistd.h>
#endif

static const char collectionGeneratorName[] = "qcollectiongenerator";
static const char helpGeneratorName[] = "qhelpgenerator";

#ifdef _WIN32
static const char separator = '\\';
#else
static const char separator = '/';
#endif

int main(int argc, char *argv[])
{
    printf("The \"%s\" tool is deprecated, use \"%s\" instead.\n\n",
           collectionGeneratorName, helpGeneratorName);

    // Replace the "qcollectiongenerator" with "qhelpgenerator"
    // in passed argv[0], keeping the path.

    const size_t currentNameSize = strlen(argv[0]);
    const size_t collectionGeneratorNameSize = strlen(collectionGeneratorName);
    const ptrdiff_t maxPathOffset = currentNameSize - collectionGeneratorNameSize;
    ptrdiff_t pathOffset = maxPathOffset;

    if (maxPathOffset >= 0 && strchr(argv[0] + maxPathOffset, separator))
        pathOffset = -1; // Separator detected. Wrong filename.

    while (pathOffset >= 0) {
        const char *fileName = argv[0] + pathOffset;

        if (fileName[0] == separator) { // Separator detected. Wrong filename.
            pathOffset = -1;
            break;
        }

        if (!strncmp(fileName, collectionGeneratorName, collectionGeneratorNameSize))
            break;

        --pathOffset;
    }

    if (pathOffset < 0) {
        fprintf(stderr, "Wrong tool name. "
                        "The tool name is expected to contain: \"%s\", got: \"%s\" instead.\n",
               collectionGeneratorName, argv[0]);
        return 3;
    }

    const size_t helpGeneratorNameSize = strlen(helpGeneratorName);
    // Allocate a buffer for the new full path, consisting of the pathSize + new name
    char *newPath = (char *) malloc((maxPathOffset + helpGeneratorNameSize + 1) * sizeof(char));
    // Copy the path
    memcpy(newPath, argv[0], pathOffset);
    // Copy the new name
    memcpy(newPath + pathOffset, helpGeneratorName, helpGeneratorNameSize);
    // Copy the remaining part
    memcpy(newPath + pathOffset + helpGeneratorNameSize,
           argv[0] + pathOffset + collectionGeneratorNameSize,
           currentNameSize - pathOffset - collectionGeneratorNameSize + 1);

    argv[0] = newPath;
#ifdef _WIN32
    const intptr_t ret = _spawnvp(_P_WAIT, newPath, argv);
    if (ret == -1) {
        fprintf(stderr, "Error while executing \"%s\" tool.\n", newPath);
        return 3;
    }
    return ret;
#else
    execvp(newPath, argv);
    fprintf(stderr, "Error while executing \"%s\" tool.\n", newPath);
    return 3;
#endif
}

