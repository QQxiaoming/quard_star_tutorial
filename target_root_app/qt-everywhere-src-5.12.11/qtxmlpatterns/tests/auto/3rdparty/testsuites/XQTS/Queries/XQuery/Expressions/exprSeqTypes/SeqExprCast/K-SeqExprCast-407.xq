(:*******************************************************:)
(: Test: K-SeqExprCast-407                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Ensure when casting xs:date to xs:string that a milli seconds part of '435' is handled properly. :)
(:*******************************************************:)
xs:string(xs:date("2002-02-15")) eq "2002-02-15"