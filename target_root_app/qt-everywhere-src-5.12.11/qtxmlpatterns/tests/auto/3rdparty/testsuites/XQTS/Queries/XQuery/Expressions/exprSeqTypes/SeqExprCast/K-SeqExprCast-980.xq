(:*******************************************************:)
(: Test: K-SeqExprCast-980                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Casting from xs:gYearMonth to xs:gYearMonth is allowed and should always succeed. :)
(:*******************************************************:)
xs:gYearMonth("1999-11") cast as xs:gYearMonth
                    eq
                  xs:gYearMonth("1999-11")