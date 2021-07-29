(:*******************************************************:)
(: Test: K-SeqExprCast-202                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Ensure that when casting xs:yearMonthDuration to xs:string, that preceding zeros are handled properly. :)
(:*******************************************************:)
xs:string(xs:yearMonthDuration("P0010Y0010M")) eq "P10Y10M"