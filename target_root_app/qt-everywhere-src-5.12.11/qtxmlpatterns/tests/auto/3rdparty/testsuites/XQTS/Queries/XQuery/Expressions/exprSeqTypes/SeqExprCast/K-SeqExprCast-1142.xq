(:*******************************************************:)
(: Test: K-SeqExprCast-1142                              :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Casting from xs:gDay to xs:gDay is allowed and should always succeed. :)
(:*******************************************************:)
xs:gDay("---03") cast as xs:gDay
                    eq
                  xs:gDay("---03")