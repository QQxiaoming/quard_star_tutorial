(:*******************************************************:)
(: Test: K-SeqExprCast-1423                              :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Casting from xs:QName to xs:untypedAtomic is allowed and should always succeed. :)
(:*******************************************************:)
xs:QName("ncname") cast as xs:untypedAtomic
                    ne
                  xs:untypedAtomic("an arbitrary string(untypedAtomic source)")