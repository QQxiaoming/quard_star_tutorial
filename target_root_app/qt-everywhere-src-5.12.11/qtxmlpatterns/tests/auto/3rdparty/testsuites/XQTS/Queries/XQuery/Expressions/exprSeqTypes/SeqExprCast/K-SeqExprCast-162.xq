(:*******************************************************:)
(: Test: K-SeqExprCast-162                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: The canonical lexical representation for the xs:duration value P3Y0M is 'P3Y'. :)
(:*******************************************************:)
xs:string(xs:yearMonthDuration("P3Y0M")) eq "P3Y"