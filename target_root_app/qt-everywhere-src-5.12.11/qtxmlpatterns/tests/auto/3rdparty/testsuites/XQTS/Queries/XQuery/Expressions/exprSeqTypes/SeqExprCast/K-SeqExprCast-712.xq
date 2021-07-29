(:*******************************************************:)
(: Test: K-SeqExprCast-712                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Casting from xs:yearMonthDuration to xs:dayTimeDuration is allowed and should always succeed. :)
(:*******************************************************:)
xs:yearMonthDuration("P1Y12M") cast as xs:dayTimeDuration
                    ne
                  xs:dayTimeDuration("P3DT2H")