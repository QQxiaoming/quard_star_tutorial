(:*******************************************************:)
(: Test: K-SeqExprCast-720                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Casting from xs:yearMonthDuration to xs:gYearMonth isn't allowed. :)
(:*******************************************************:)
xs:yearMonthDuration("P1Y12M") cast as xs:gYearMonth