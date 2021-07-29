(:*******************************************************:)
(: Test: K-SeqExprCast-1316                              :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: A simple test exercising the whitespace facet for type xs:hexBinary. :)
(:*******************************************************:)
xs:hexBinary("
	 0FB7
	 ")
        eq
        xs:hexBinary("
	 0FB7
	 ")
      