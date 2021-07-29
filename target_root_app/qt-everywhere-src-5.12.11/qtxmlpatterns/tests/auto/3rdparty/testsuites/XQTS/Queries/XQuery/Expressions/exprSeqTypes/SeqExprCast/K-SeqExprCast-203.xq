(:*******************************************************:)
(: Test: K-SeqExprCast-203                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Ensure that a negative xs:yearMonthDuration is properly serialized when cast to xs:string. :)
(:*******************************************************:)
xs:string(xs:yearMonthDuration("-P0010Y0010M")) eq "-P10Y10M"