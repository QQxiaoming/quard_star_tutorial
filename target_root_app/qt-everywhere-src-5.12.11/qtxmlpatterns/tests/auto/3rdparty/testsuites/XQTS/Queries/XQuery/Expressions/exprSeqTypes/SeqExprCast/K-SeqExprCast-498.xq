(:*******************************************************:)
(: Test: K-SeqExprCast-498                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Casting from xs:double to xs:double is allowed and should always succeed. :)
(:*******************************************************:)
xs:double("3.3e3") cast as xs:double
                    eq
                  xs:double("3.3e3")