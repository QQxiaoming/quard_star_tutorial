(:*******************************************************:)
(: Test: K-SeqExprCast-900                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: A simple test exercising the whitespace facet for type xs:date. :)
(:*******************************************************:)
xs:date("
	 2004-10-13
	 ")
        eq
        xs:date("
	 2004-10-13
	 ")
      