(:*******************************************************:)
(: Test: K-SeqExprCast-750                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Casting from xs:dayTimeDuration to xs:string is allowed and should always succeed. :)
(:*******************************************************:)
xs:dayTimeDuration("P3DT2H") cast as xs:string
                    ne
                  xs:string("an arbitrary string")