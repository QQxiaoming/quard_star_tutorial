(:*******************************************************:)
(: Test: K-SeqExprCast-1264                              :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: A simple test exercising the whitespace facet for type xs:base64Binary. :)
(:*******************************************************:)
xs:base64Binary("
	 aaaa
	 ")
        eq
        xs:base64Binary("
	 aaaa
	 ")
      