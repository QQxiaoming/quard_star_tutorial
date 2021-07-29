(:*******************************************************:)
(: Test: K-SeqExprCast-768                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Casting from xs:dayTimeDuration to xs:time isn't allowed. :)
(:*******************************************************:)
xs:dayTimeDuration("P3DT2H") cast as xs:time