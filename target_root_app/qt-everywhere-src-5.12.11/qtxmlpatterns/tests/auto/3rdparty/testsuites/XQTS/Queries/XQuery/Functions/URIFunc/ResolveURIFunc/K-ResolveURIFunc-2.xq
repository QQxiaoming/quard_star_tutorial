(:*******************************************************:)
(: Test: K-ResolveURIFunc-2                              :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: A test whose essence is: `resolve-uri("http://www.example.com/", "relative/uri.ext", "wrong param")`. :)
(:*******************************************************:)
resolve-uri("http://www.example.com/", 
				"relative/uri.ext", "wrong param")