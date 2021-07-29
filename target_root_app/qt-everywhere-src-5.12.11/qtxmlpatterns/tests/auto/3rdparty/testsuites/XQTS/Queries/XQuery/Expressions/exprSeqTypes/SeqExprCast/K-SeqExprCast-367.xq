(:*******************************************************:)
(: Test: K-SeqExprCast-367                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Ensure when casting xs:time to xs:string that an empty milli seconds part is not serialized. :)
(:*******************************************************:)
xs:string(xs:time("21:01:23.000")) eq "21:01:23"