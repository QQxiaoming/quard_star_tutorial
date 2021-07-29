(:*******************************************************:)
(: Test: K-SeqExprCast-710                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Casting from xs:yearMonthDuration to xs:yearMonthDuration is allowed and should always succeed. :)
(:*******************************************************:)
xs:yearMonthDuration("P1Y12M") cast as xs:yearMonthDuration
                    eq
                  xs:yearMonthDuration("P1Y12M")