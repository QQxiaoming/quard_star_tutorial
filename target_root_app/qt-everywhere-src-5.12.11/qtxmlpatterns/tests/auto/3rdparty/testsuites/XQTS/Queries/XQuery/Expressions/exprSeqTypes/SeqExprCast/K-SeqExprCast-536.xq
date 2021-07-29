(:*******************************************************:)
(: Test: K-SeqExprCast-536                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: A simple test exercising the whitespace facet for type xs:decimal. :)
(:*******************************************************:)
xs:decimal("
	 10.01
	 ")
        eq
        xs:decimal("
	 10.01
	 ")
      