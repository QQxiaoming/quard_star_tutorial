(:*******************************************************:)
(: Test: K-GenCompEq-55                                  :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: General comparison which fails due to invalid operator combination or casting. :)
(:*******************************************************:)

		(xs:untypedAtomic("1"), xs:anyURI("example.com")) =
		(xs:untypedAtomic("2.0"), 3.0)