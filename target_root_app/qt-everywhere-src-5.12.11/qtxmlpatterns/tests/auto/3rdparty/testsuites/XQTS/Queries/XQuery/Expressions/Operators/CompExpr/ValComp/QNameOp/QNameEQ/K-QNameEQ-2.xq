(:*******************************************************:)
(: Test: K-QNameEQ-2                                     :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: A test whose essence is: `QName("example.com/", "p:ncname") eq QName("example.com/", "pdifferent:ncname")`. :)
(:*******************************************************:)
QName("example.com/", "p:ncname") eq
			       QName("example.com/", "pdifferent:ncname")