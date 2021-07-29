(:*******************************************************:)
(: Test: K-SeqExprCast-1196                              :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Casting from xs:gMonth to xs:gMonth is allowed and should always succeed. :)
(:*******************************************************:)
xs:gMonth("--11") cast as xs:gMonth
                    eq
                  xs:gMonth("--11")