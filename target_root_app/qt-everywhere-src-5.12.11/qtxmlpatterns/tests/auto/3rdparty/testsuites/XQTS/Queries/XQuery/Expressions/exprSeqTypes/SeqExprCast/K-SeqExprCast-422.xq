(:*******************************************************:)
(: Test: K-SeqExprCast-422                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Casting from xs:untypedAtomic to xs:QName isn't allowed. :)
(:*******************************************************:)
xs:untypedAtomic("an arbitrary string(untypedAtomic source)") cast as xs:QName