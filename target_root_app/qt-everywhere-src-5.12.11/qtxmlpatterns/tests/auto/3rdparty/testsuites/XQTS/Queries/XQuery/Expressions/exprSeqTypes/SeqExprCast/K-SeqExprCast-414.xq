(:*******************************************************:)
(: Test: K-SeqExprCast-414                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: A simple test exercising the whitespace facet for type xs:untypedAtomic. :)
(:*******************************************************:)
xs:untypedAtomic("
	 an arbitrary string(untypedAtomic source)
	 ")
        eq
        xs:untypedAtomic("
	 an arbitrary string(untypedAtomic source)
	 ")
      