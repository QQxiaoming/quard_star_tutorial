(:*******************************************************:)
(: Test: K-SeqExprCast-1320                              :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Casting from xs:hexBinary to xs:untypedAtomic is allowed and should always succeed. :)
(:*******************************************************:)
xs:hexBinary("0FB7") cast as xs:untypedAtomic
                    ne
                  xs:untypedAtomic("an arbitrary string(untypedAtomic source)")