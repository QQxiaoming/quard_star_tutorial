(:*******************************************************:)
(: Test: K-SeqExprCast-1008                              :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Casting from xs:gYear to xs:untypedAtomic is allowed and should always succeed. :)
(:*******************************************************:)
xs:gYear("1999") cast as xs:untypedAtomic
                    ne
                  xs:untypedAtomic("an arbitrary string(untypedAtomic source)")