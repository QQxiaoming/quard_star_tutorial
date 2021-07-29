(:*******************************************************:)
(: Test: K-QNameEQ-3                                     :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: A test whose essence is: `QName("example.com/", "p:ncname") ne QName("example.com/Nope", "p:ncname")`. :)
(:*******************************************************:)
QName("example.com/", "p:ncname") ne
			       QName("example.com/Nope", "p:ncname")