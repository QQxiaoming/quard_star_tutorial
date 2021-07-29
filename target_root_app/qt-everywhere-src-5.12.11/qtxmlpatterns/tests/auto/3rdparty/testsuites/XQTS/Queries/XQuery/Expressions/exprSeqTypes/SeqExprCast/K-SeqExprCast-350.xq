(:*******************************************************:)
(: Test: K-SeqExprCast-350                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: The canonical form of an xs:dateTime containing 24:00:00 is the following day at 00:00:00. :)
(:*******************************************************:)
string(xs:dateTime("2004-03-31T24:00:00")) eq "2004-04-01T00:00:00"