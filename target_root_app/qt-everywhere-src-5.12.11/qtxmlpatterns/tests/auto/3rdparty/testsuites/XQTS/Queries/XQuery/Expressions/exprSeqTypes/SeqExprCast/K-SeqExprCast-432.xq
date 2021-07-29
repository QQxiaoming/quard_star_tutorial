(:*******************************************************:)
(: Test: K-SeqExprCast-432                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Casting from xs:string to xs:untypedAtomic is allowed and should always succeed. :)
(:*******************************************************:)
xs:string("an arbitrary string") cast as xs:untypedAtomic
                    ne
                  xs:untypedAtomic("an arbitrary string(untypedAtomic source)")