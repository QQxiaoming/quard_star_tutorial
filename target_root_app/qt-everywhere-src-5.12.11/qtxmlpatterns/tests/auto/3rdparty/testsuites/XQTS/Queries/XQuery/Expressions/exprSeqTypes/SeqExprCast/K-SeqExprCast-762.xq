(:*******************************************************:)
(: Test: K-SeqExprCast-762                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Casting from xs:dayTimeDuration to xs:yearMonthDuration is allowed and should always succeed. :)
(:*******************************************************:)
xs:dayTimeDuration("P3DT2H") cast as xs:yearMonthDuration
                    ne
                  xs:yearMonthDuration("P1Y12M")