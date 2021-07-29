(:*******************************************************:)
(: Test: K-SeqExprCast-546                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Casting from xs:decimal to xs:double is allowed and should always succeed. :)
(:*******************************************************:)
xs:decimal("10.01") cast as xs:double
                    ne
                  xs:double("3.3e3")