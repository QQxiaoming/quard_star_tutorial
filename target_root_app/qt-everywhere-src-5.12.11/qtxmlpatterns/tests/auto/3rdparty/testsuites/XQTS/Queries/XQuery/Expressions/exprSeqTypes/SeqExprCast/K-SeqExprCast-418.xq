(:*******************************************************:)
(: Test: K-SeqExprCast-418                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Casting from xs:untypedAtomic to xs:untypedAtomic is allowed and should always succeed. :)
(:*******************************************************:)
xs:untypedAtomic("an arbitrary string(untypedAtomic source)") cast as xs:untypedAtomic
                    eq
                  xs:untypedAtomic("an arbitrary string(untypedAtomic source)")