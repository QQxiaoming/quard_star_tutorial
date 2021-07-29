(:*******************************************************:)
(: Test: K-SeqExprCast-656                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Casting from xs:duration to xs:duration is allowed and should always succeed. :)
(:*******************************************************:)
xs:duration("P1Y2M3DT10H30M") cast as xs:duration
                    eq
                  xs:duration("P1Y2M3DT10H30M")