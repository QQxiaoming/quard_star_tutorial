(:*******************************************************:)
(: Test: K-SeqExprCast-760                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Casting from xs:dayTimeDuration to xs:duration is allowed and should always succeed. :)
(:*******************************************************:)
xs:dayTimeDuration("P3DT2H") cast as xs:duration
                    ne
                  xs:duration("P1Y2M3DT10H30M")