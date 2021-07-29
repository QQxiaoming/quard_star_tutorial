(:*******************************************************:)
(: Test: K-SeqExprCast-1268                              :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Casting from xs:base64Binary to xs:untypedAtomic is allowed and should always succeed. :)
(:*******************************************************:)
xs:base64Binary("aaaa") cast as xs:untypedAtomic
                    ne
                  xs:untypedAtomic("an arbitrary string(untypedAtomic source)")