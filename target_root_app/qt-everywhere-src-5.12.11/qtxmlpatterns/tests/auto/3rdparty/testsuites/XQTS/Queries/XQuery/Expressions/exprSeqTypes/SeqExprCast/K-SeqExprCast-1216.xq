(:*******************************************************:)
(: Test: K-SeqExprCast-1216                              :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Casting from xs:boolean to xs:untypedAtomic is allowed and should always succeed. :)
(:*******************************************************:)
xs:boolean("true") cast as xs:untypedAtomic
                    ne
                  xs:untypedAtomic("an arbitrary string(untypedAtomic source)")