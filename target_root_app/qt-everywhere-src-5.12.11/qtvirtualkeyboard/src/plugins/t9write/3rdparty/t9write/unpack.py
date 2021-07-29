#!/usr/bin/env python
# -*- coding: utf-8 -*-

#############################################################################
##
## Copyright (C) 2017 The Qt Company Ltd.
## Contact: https://www.qt.io/licensing/
##
## This file is part of the Qt Virtual Keyboard module of the Qt Toolkit.
##
## $QT_BEGIN_LICENSE:GPL$
## Commercial License Usage
## Licensees holding valid commercial Qt licenses may use this file in
## accordance with the commercial license agreement provided with the
## Software or, alternatively, in accordance with the terms contained in
## a written agreement between you and The Qt Company. For licensing terms
## and conditions see https://www.qt.io/terms-conditions. For further
## information use the contact form at https://www.qt.io/contact-us.
##
## GNU General Public License Usage
## Alternatively, this file may be used under the terms of the GNU
## General Public License version 3 or (at your option) any later version
## approved by the KDE Free Qt Foundation. The licenses are as published by
## the Free Software Foundation and appearing in the file LICENSE.GPL3
## included in the packaging of this file. Please review the following
## information to ensure the GNU General Public License requirements will
## be met: https://www.gnu.org/licenses/gpl-3.0.html.
##
## $QT_END_LICENSE$
##
#############################################################################

import os
import sys
import zipfile
import tempfile
import shutil
import fnmatch

#
# This utility script unpacks the T9 Write SDK to appropriate directory
# structure for Qt Virtual Keyboard.
#
# Usage: unpack.py <filename.zip> <target dir>
#        unpack.py <input dir> <target dir>
#
# The script will happily overwrite existing files, so be careful.
#

#
# Unpack rule list
#
# Each list entry is a dictionary consisting of target directory as
# key and matching pattern as value. The dictionary can be defined in
# the following ways:
#
# Note: The rules within the dictionary are executed in arbitrary order.
#       Add a new list entry if the order is significant.
#
# Format:
#   1. { 'target dir 1': [ 'pattern1', 'pattern2', ... ], 'target dir 2': ... }
#       - Each pattern is matched against the zip file contents. The file is
#         copied to target dir if the pattern matches. Each pattern is handled
#         independent of each other.
#
#   2. { 'target dir 1': [ [ 'file group pattern', 'sub pattern1', ... ] ], 'target dir 2': ... }
#       - First the file group pattern is matched against the zip file contents.
#         Then all the sub patterns are matched in the sub directory specified by
#         the first match. If all the sub patterns match, then first match from
#         file group pattern and all the matching files from sub pattterns are copied.
#         The purpose of this option is to copy coupled files, e.g. DLL and LIB
#         files found in the same directory.
#

UNPACK_RULES = [
{ # Header files
'api': [
    '*/decuma_hwr.h',
    '*/decuma_hwr_cjk.h',
    '*/decuma_hwr_types.h',
    '*/decuma_point.h',
    '*/decumaBasicTypes.h',
    '*/decumaBasicTypesMinMax.h',
    '*/decumaCharacterSetType.h',
    '*/decumaCurve.h',
    '*/decumaFunctionalSupport.h',
    '*/decumaFunctionalSupportCheck.h',
    '*/decumaLanguages.h',
    '*/decumaLiteFunctionalSupport.h',
    '*/decumaPlusFunctionalSupport.h',
    '*/decumaRuntimeMallocData.h',
    '*/decumaStatus.h',
    '*/decumaStorageSpecifiers.h',
    '*/decumaSymbolCategories.h',
    '*/decumaUnicodeTypes.h',
    '*/t9write_alpha_version.h',
    '*/t9write_api_version.h',
    '*/t9write_cjk_version.h',
    '*/xxt9wApiOem.h',
    '*/xxt9wOem.h',
],
}, { # Data: Arabic and Hebrew database must be copied first (the file name collides with Alphabetic database)
'data/arabic': [
    '*/Arabic/*_le.bin',
],
'data/hebrew': [
    '*/Hebrew/*_le.bin',
],
'data/thai': [
    '*/*Thai*/*_le.bin',
],
}, { # Data: Alphabetic and CJK databases
'data': [
    '*/*_le.bin',
    '*/*.hdb',
    '*/*.phd',
    '*/*.ldb',
],
}, { # Libraries
'lib/arm/static/alphabetic': [
    '*T9Write_Alpha*/*Android_ARM*/*.a',
    '*T9Write_Alpha*/*Android_ARM*/*.o',
],
'lib/arm/shared/alphabetic': [
    '*T9Write_Alpha*/*Android_ARM*/*.so',
],
'lib/arm/static/cjk': [
    '*T9Write_CJK*/*Android_ARM*/*.a',
    '*T9Write_CJK*/*Android_ARM*/*.o',
],
'lib/arm/shared/cjk': [
    '*T9Write_CJK*/*Android_ARM*/*.so',
],
'lib/linux/static/alphabetic': [
    '*T9Write_Alpha*/*Android_x86*/*.a',
    '*T9Write_Alpha*/*Android_x86*/*.o',
],
'lib/linux/shared/alphabetic': [
    '*T9Write_Alpha*/*Android_x86*/*.so',
],
'lib/linux/static/cjk': [
    '*T9Write_CJK*/*Android_x86*/*.a',
    '*T9Write_CJK*/*Android_x86*/*.o',
],
'lib/linux/shared/cjk': [
    '*T9Write_CJK*/*Android_x86*/*.so',
],
'lib/win32/static/alphabetic': [
    '*T9Write_Alpha*/*.obj',
],
'lib/win32/shared/alphabetic': [
    [ '*T9Write_Alpha*/*.dll', '*.lib' ],
],
'lib/win32/static/cjk': [
    '*T9Write_CJK*/*.obj',
],
'lib/win32/shared/cjk': [
    [ '*T9Write_CJK*/*.dll', '*.lib' ],
],
'lib/win32/shared/alphabetic': [
    [ '*T9Write_Alpha*/*.dll', '*.lib' ],
],
}
]

#
# Blacklist
#
# File matching rules for blacklisted items. Matched before UNPACK_RULES.
#

BLACKLIST_RULES = [
'*__MACOSX*',
'*/.DS_Store',
]

def blacklist(file_list):
    result = []
    for file_name in file_list:
        match = False
        for blacklist_rule in BLACKLIST_RULES:
            match = fnmatch.fnmatch(file_name, blacklist_rule)
            if match:
                break
        if not match:
            result.append(file_name)
    return result

def unzip(zip_file, target_dir):
    zip_list = []
    if os.path.isdir(zip_file):
        base_dir, sdk_dir = os.path.split(zip_file.replace('\\', '/').rstrip('/'))
        base_dir_length = len(base_dir) + 1 if base_dir else 0
        if not 'T9Write' in sdk_dir:
            print("Error: The input directory name '" + sdk_dir + "' does not contain 'T9Write'.")
            print("Please unzip the file to a directory named after the zip file and try again.")
            return zip_list
        for root, dirs, files in os.walk(zip_file):
            for file_name in files:
                sub_dir = root[base_dir_length:]
                dst_dir = os.path.join(target_dir, sub_dir)
                if not os.path.exists(dst_dir):
                    os.makedirs(dst_dir)
                shutil.copy2(os.path.join(root, file_name), dst_dir)
                os.chmod(os.path.join(dst_dir, file_name), 0o644)
                zip_list.append(os.path.join(sub_dir, file_name).replace('\\', '/'))
        return zip_list
    with zipfile.ZipFile(zip_file, 'r') as z:
        zip_list = sorted(blacklist(z.namelist()))
        zip_basename = os.path.splitext(os.path.basename(zip_file))[0]
        if zip_list and zip_basename in zip_list[0]:
            zip_basename = ''
        zip_list = [os.path.join(zip_basename, zip_name).replace('\\', '/') for zip_name in zip_list]
        z.extractall(os.path.join(target_dir, zip_basename))
    return zip_list

def match_file_list(file_list, base_dir, fnpattern):
    return [file_name for file_name in file_list \
        if fnmatch.fnmatch(file_name, fnpattern) and \
            os.path.isfile(os.path.join(base_dir, file_name))]

def unpack(zip_list, zip_dir, out_dir):
    if not zip_list:
        return

    for unpack_rules in UNPACK_RULES:
        process_unpack_rules(zip_list, zip_dir, out_dir, unpack_rules)

def process_unpack_rules(zip_list, zip_dir, out_dir, unpack_rules):
    for (target_dir, match_rules) in unpack_rules.items():
        for match_rule in match_rules:
            # Match
            match_rule_group = match_rule if isinstance(match_rule, list) else [match_rule]
            match_group_candidates = [match_file_list(zip_list, zip_dir, match_rule_group[0])]
            if len(match_rule_group) > 1:
                while len(match_group_candidates[0]) > 0:
                    match_group0_candidate = match_group_candidates[0][0]
                    all_sub_groups_match = True
                    for sub_group_rule in match_rule_group[1:]:
                        fnpattern = os.path.join(os.path.dirname(match_group0_candidate), sub_group_rule).replace('\\', '/')
                        sub_group_candidates = match_file_list(zip_list, zip_dir, fnpattern)
                        if not sub_group_candidates:
                            all_sub_groups_match = False
                            break
                        match_group_candidates.append(sub_group_candidates)
                    if all_sub_groups_match:
                        match_group_candidates[0] = [match_group0_candidate]
                        break
                    else:
                        match_group_candidates = [match_group_candidates[0][1:]]

            # Copy
            if match_group_candidates:
                for match_group_candidate in match_group_candidates:
                    for zip_name in match_group_candidate:
                        dst_dir = os.path.join(out_dir, target_dir)
                        if not os.path.exists(dst_dir):
                            os.makedirs(dst_dir)
                        src = os.path.join(zip_dir, zip_name).replace('\\', '/')
                        dst = os.path.join(dst_dir, os.path.basename(zip_name)).replace('\\', '/')
                        print(zip_name + ' -> ' + dst)
                        shutil.copy2(src, dst)
                        zip_list.remove(zip_name)

if __name__ == '__main__':
    if len(sys.argv) != 3:
        print("Usage: %s <filename.zip> <target dir>" % os.path.basename(__file__))
        exit()

    out_dir = sys.argv[2]
    zip_dir = tempfile.mkdtemp()

    try:
        unpack(unzip(sys.argv[1], zip_dir), zip_dir, out_dir)
    except Exception as e:
        print(e)
    finally:
        shutil.rmtree(zip_dir)
