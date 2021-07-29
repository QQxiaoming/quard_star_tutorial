(:*******************************************************:)
(: Test: K-SeqExprCast-1358                              :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Casting from xs:hexBinary to xs:hexBinary is allowed and should always succeed. :)
(:*******************************************************:)
xs:hexBinary("0FB7") cast as xs:hexBinary
                    eq
                  xs:hexBinary("0FB7")