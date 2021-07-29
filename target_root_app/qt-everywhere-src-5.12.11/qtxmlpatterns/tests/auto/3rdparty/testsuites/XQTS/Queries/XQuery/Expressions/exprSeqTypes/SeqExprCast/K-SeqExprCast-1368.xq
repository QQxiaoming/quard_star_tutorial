(:*******************************************************:)
(: Test: K-SeqExprCast-1368                              :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: A simple test exercising the whitespace facet for type xs:anyURI. :)
(:*******************************************************:)
xs:anyURI("
	 http://www.example.com/an/arbitrary/URI.ext
	 ")
        eq
        xs:anyURI("
	 http://www.example.com/an/arbitrary/URI.ext
	 ")
      