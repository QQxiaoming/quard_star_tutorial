(:*******************************************************:)
(: Test: K-SeqExprCast-1034                              :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Casting from xs:gYear to xs:gYear is allowed and should always succeed. :)
(:*******************************************************:)
xs:gYear("1999") cast as xs:gYear
                    eq
                  xs:gYear("1999")