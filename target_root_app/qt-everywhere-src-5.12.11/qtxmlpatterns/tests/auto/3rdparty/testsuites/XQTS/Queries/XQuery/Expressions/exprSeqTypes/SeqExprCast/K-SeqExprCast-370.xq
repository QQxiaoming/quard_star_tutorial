(:*******************************************************:)
(: Test: K-SeqExprCast-370                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Ensure when casting xs:time to xs:string that milli seconds are properly padded with zeros. :)
(:*******************************************************:)
xs:string(xs:time("21:01:23.010")) eq "21:01:23.01"