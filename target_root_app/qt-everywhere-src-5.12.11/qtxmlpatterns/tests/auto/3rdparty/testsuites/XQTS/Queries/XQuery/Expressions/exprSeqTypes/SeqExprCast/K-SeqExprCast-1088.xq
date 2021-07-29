(:*******************************************************:)
(: Test: K-SeqExprCast-1088                              :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Casting from xs:gMonthDay to xs:gMonthDay is allowed and should always succeed. :)
(:*******************************************************:)
xs:gMonthDay("--11-13") cast as xs:gMonthDay
                    eq
                  xs:gMonthDay("--11-13")