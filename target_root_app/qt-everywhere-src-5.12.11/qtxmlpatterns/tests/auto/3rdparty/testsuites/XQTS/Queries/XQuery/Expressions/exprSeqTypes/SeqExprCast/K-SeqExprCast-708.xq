(:*******************************************************:)
(: Test: K-SeqExprCast-708                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Casting from xs:yearMonthDuration to xs:duration is allowed and should always succeed. :)
(:*******************************************************:)
xs:yearMonthDuration("P1Y12M") cast as xs:duration
                    ne
                  xs:duration("P1Y2M3DT10H30M")