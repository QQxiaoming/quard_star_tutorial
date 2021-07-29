(:*******************************************************:)
(: Test: K2-FunctionProlog-28                            :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Compute the levenshtein distance between strings. :)
(:*******************************************************:)
declare function local:levenshtein($arg1 as xs:string, $arg2 as xs:string) as xs:decimal
{
    if(string-length($arg1) = 0)
    then string-length($arg2)
    else if(string-length($arg2) = 0)
         then string-length($arg1)
         else min((local:levenshtein(substring($arg1, 2), $arg2) + 1,
                   local:levenshtein($arg1, substring($arg2, 2)) + 1,
                   local:levenshtein(substring($arg1, 2), substring($arg2, 2)) +
                   (if(substring($arg1, 1, 1) = substring($arg2, 1, 1))
                    then 0
                    else 1)))
};

local:levenshtein("a", "a"),
local:levenshtein("aa", "aa"),
local:levenshtein("aaa", "aaa"),
local:levenshtein("aa a", "aa a"),
local:levenshtein("a a a", "aaa"),
local:levenshtein("aaa", "a a a"),
local:levenshtein("aaa", "aaab"),
local:levenshtein("978", "abc")