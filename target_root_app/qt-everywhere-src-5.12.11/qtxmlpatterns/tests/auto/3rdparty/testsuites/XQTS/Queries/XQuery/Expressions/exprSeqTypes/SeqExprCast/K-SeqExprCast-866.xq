(:*******************************************************:)
(: Test: K-SeqExprCast-866                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Casting from xs:time to xs:yearMonthDuration isn't allowed. :)
(:*******************************************************:)
xs:time("03:20:00-05:00") cast as xs:yearMonthDuration