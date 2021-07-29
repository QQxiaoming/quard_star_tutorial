(:*******************************************************:)
(: Test: K-SeqExprCast-588                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: A simple test exercising the whitespace facet for type xs:integer. :)
(:*******************************************************:)
xs:integer("
	 6789
	 ")
        eq
        xs:integer("
	 6789
	 ")
      