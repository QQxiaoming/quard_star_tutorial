(:*******************************************************:)
(: Test: K-SeqExprCast-958                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Casting from xs:gYearMonth to xs:string is allowed and should always succeed. :)
(:*******************************************************:)
xs:gYearMonth("1999-11") cast as xs:string
                    ne
                  xs:string("an arbitrary string")