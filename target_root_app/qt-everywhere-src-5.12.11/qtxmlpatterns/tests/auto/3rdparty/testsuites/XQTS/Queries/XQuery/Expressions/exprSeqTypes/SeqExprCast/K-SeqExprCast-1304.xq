(:*******************************************************:)
(: Test: K-SeqExprCast-1304                              :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Casting from xs:base64Binary to xs:base64Binary is allowed and should always succeed. :)
(:*******************************************************:)
xs:base64Binary("aaaa") cast as xs:base64Binary
                    eq
                  xs:base64Binary("aaaa")