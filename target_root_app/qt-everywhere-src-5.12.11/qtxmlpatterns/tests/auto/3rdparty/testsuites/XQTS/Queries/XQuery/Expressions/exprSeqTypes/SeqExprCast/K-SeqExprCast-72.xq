(:*******************************************************:)
(: Test: K-SeqExprCast-72                                :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: A test whose essence is: `"example.com/" cast as xs:anyURI eq xs:anyURI('example.com/')`. :)
(:*******************************************************:)
"example.com/" cast as xs:anyURI eq 
				xs:anyURI('example.com/')