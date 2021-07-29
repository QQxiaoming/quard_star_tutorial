(:*******************************************************:)
(: Test: K-SeqExprCast-420                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Casting from xs:untypedAtomic to xs:string is allowed and should always succeed. :)
(:*******************************************************:)
xs:untypedAtomic("an arbitrary string(untypedAtomic source)") cast as xs:string
                    ne
                  xs:string("an arbitrary string")