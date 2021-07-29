(:*******************************************************:)
(: Test: K-SeqExprCast-450                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Casting from xs:float to xs:double is allowed and should always succeed. :)
(:*******************************************************:)
xs:float("3.4e5") cast as xs:double
                    ne
                  xs:double("3.3e3")