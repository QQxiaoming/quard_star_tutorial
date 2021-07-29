(:*******************************************************:)
(: Test: K-SeqExprCast-364                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Simple xs:time involving time with no milli seconds. :)
(:*******************************************************:)
string(xs:time("12:44:05")) eq "12:44:05"