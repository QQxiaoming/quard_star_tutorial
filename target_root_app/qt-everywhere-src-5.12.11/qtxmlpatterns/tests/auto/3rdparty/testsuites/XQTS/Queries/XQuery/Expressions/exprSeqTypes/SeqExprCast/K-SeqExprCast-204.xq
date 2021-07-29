(:*******************************************************:)
(: Test: K-SeqExprCast-204                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Ensure that when casting xs:yearMonthDuration to xs:string, that empty fields are properly serialized. :)
(:*******************************************************:)
xs:string(xs:yearMonthDuration("P0Y0M")) eq "P0M"