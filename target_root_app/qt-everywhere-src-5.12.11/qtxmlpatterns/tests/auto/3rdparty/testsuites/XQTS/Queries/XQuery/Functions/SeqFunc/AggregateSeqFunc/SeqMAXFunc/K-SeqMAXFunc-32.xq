(:*******************************************************:)
(: Test: K-SeqMAXFunc-32                                 :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:23+01:00                       :)
(: Purpose: A test whose essence is: `max((xs:float(-3), xs:untypedAtomic("3"), xs:double("NaN"))) instance of xs:double`. :)
(:*******************************************************:)
max((xs:float(-3), xs:untypedAtomic("3"), xs:double("NaN")))
			instance of xs:double