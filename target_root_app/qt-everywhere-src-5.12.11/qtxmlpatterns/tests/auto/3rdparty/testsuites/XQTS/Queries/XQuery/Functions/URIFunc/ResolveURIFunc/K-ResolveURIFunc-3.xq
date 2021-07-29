(:*******************************************************:)
(: Test: K-ResolveURIFunc-3                              :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: A test whose essence is: `resolve-uri("relative/uri.ext", "http://www.example.com/") eq xs:anyURI("http://www.example.com/relative/uri.ext")`. :)
(:*******************************************************:)
resolve-uri("relative/uri.ext", "http://www.example.com/") eq
			xs:anyURI("http://www.example.com/relative/uri.ext")