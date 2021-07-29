(:*******************************************************:)
(: Test: K-SeqExprCast-1220                              :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Casting from xs:boolean to xs:float is allowed and should always succeed. :)
(:*******************************************************:)
xs:boolean("true") cast as xs:float
                    ne
                  xs:float("3.4e5")