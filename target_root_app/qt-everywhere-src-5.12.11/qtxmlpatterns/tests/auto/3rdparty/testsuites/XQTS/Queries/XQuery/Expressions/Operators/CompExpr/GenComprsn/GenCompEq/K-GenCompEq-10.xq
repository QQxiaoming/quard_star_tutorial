(:*******************************************************:)
(: Test: K-GenCompEq-10                                  :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Example from the XPath 2.0 specification.    :)
(:*******************************************************:)
not((xs:untypedAtomic("1"), xs:untypedAtomic("2")) =
				   (xs:untypedAtomic("2.0"), 3.0))