(:*******************************************************:)
(: Test: K-SeqExprCast-374                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: The canonical form of an xs:time containing 24:00:00 is 00:00:00. :)
(:*******************************************************:)
string(xs:time("24:00:00")) eq "00:00:00"