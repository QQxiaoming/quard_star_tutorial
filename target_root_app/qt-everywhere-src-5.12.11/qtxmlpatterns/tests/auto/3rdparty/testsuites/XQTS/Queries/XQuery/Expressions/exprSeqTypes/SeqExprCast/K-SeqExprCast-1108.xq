(:*******************************************************:)
(: Test: K-SeqExprCast-1108                              :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: A simple test exercising the whitespace facet for type xs:gDay. :)
(:*******************************************************:)
xs:gDay("
	 ---03
	 ")
        eq
        xs:gDay("
	 ---03
	 ")
      