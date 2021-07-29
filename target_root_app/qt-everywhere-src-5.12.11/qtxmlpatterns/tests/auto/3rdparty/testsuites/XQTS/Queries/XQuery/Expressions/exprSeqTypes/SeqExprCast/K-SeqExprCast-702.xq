(:*******************************************************:)
(: Test: K-SeqExprCast-702                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Casting from xs:yearMonthDuration to xs:double isn't allowed. :)
(:*******************************************************:)
xs:yearMonthDuration("P1Y12M") cast as xs:double