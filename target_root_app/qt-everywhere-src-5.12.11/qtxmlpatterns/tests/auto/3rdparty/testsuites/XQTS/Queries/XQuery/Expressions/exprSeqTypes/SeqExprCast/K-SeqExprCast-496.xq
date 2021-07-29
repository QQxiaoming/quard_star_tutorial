(:*******************************************************:)
(: Test: K-SeqExprCast-496                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Casting from xs:double to xs:float is allowed and should always succeed. :)
(:*******************************************************:)
xs:double("3.3e3") cast as xs:float
                    ne
                  xs:float("3.4e5")