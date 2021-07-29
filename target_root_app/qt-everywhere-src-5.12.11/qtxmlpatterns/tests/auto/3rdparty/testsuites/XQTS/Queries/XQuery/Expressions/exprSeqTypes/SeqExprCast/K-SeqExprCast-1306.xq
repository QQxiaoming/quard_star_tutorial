(:*******************************************************:)
(: Test: K-SeqExprCast-1306                              :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Casting from xs:base64Binary to xs:hexBinary is allowed and should always succeed. :)
(:*******************************************************:)
xs:base64Binary("aaaa") cast as xs:hexBinary
                    ne
                  xs:hexBinary("0FB7")