(:*******************************************************:)
(: Test: K-SeqExprCast-50                                :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: A test whose essence is: `false() cast as xs:untypedAtomic eq xs:untypedAtomic("false")`. :)
(:*******************************************************:)
false() cast as xs:untypedAtomic 
		eq xs:untypedAtomic("false")