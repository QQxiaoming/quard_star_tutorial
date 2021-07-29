(:*******************************************************:)
(: Test: K2-SeqSubsequenceFunc-1                         :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Use fn:subsequence where the input is via variable references. :)
(:*******************************************************:)
declare variable $start as xs:integer := (current-time(), 2)[2] treat as xs:integer;
declare variable $len as xs:integer := (current-time(), 1)[2] treat as xs:integer;
subsequence((1, 2, 3), $start, $len)