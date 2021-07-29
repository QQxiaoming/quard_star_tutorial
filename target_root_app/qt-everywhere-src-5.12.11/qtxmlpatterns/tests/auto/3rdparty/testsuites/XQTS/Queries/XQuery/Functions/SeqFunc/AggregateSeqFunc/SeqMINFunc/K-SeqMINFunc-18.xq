(:*******************************************************:)
(: Test: K-SeqMINFunc-18                                 :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:23+01:00                       :)
(: Purpose: A test whose essence is: `min((xs:float("NaN"), xs:untypedAtomic("3"), xs:double(2))) instance of xs:double`. :)
(:*******************************************************:)
min((xs:float("NaN"), xs:untypedAtomic("3"), xs:double(2)))
			instance of xs:double