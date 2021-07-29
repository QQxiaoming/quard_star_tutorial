(:*******************************************************:)
(: Test: K-NumericSubtract-33                            :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:20+01:00                       :)
(: Purpose: A test whose essence is: `(xs:untypedAtomic("3") - xs:untypedAtomic("3")) instance of xs:double`. :)
(:*******************************************************:)
(xs:untypedAtomic("3") -
			       xs:untypedAtomic("3")) instance of xs:double