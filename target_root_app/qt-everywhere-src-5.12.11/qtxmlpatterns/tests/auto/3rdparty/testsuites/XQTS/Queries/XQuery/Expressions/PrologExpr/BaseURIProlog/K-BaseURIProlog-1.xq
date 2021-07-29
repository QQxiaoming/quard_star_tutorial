(:*******************************************************:)
(: Test: K-BaseURIProlog-1                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Test 'declare base-uri' with fn:static-base-uri(). :)
(:*******************************************************:)

	(::)declare(::)base-uri(::)"http://example.com/declareBaseURITest"(::);
		static-base-uri() eq 'http://example.com/declareBaseURITest'
	