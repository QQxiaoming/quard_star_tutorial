(:*******************************************************:)
(: Test: K-SeqExprCast-544                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Casting from xs:decimal to xs:float is allowed and should always succeed. :)
(:*******************************************************:)
xs:decimal("10.01") cast as xs:float
                    ne
                  xs:float("3.4e5")