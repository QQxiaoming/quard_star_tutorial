(:*******************************************************:)
(: Test: K-SeqExprCast-1356                              :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Casting from xs:hexBinary to xs:base64Binary is allowed and should always succeed. :)
(:*******************************************************:)
xs:hexBinary("0FB7") cast as xs:base64Binary
                    ne
                  xs:base64Binary("aaaa")