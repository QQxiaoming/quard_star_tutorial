(:*******************************************************:)
(: Test: K-SeqExprCast-488                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: A simple test exercising the whitespace facet for type xs:double. :)
(:*******************************************************:)
xs:double("
	 3.3e3
	 ")
        eq
        xs:double("
	 3.3e3
	 ")
      