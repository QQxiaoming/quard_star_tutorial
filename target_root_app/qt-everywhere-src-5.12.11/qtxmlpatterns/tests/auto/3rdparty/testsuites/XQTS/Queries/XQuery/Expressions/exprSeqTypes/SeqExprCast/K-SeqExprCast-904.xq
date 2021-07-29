(:*******************************************************:)
(: Test: K-SeqExprCast-904                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Casting from xs:date to xs:untypedAtomic is allowed and should always succeed. :)
(:*******************************************************:)
xs:date("2004-10-13") cast as xs:untypedAtomic
                    ne
                  xs:untypedAtomic("an arbitrary string(untypedAtomic source)")