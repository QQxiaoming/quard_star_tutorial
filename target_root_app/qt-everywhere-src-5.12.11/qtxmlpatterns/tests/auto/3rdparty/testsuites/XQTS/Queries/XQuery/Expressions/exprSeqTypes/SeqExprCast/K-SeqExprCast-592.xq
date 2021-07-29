(:*******************************************************:)
(: Test: K-SeqExprCast-592                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Casting from xs:integer to xs:untypedAtomic is allowed and should always succeed. :)
(:*******************************************************:)
xs:integer("6789") cast as xs:untypedAtomic
                    ne
                  xs:untypedAtomic("an arbitrary string(untypedAtomic source)")