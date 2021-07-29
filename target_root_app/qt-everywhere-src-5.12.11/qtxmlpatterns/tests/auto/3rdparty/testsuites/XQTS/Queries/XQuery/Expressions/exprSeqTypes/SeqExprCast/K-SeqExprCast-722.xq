(:*******************************************************:)
(: Test: K-SeqExprCast-722                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Casting from xs:yearMonthDuration to xs:gYear isn't allowed. :)
(:*******************************************************:)
xs:yearMonthDuration("P1Y12M") cast as xs:gYear