(:*******************************************************:)
(: Test: K-SeqExprCast-428                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: A simple test exercising the whitespace facet for type xs:string. :)
(:*******************************************************:)
xs:string("
	 an arbitrary string
	 ")
        eq
        xs:string("
	 an arbitrary string
	 ")
      