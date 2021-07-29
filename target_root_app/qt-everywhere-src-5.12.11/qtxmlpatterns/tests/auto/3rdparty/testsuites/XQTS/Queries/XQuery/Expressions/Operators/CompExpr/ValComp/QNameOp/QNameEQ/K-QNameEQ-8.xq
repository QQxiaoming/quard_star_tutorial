(:*******************************************************:)
(: Test: K-QNameEQ-8                                     :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Operator 'le' is not available between values of type xs:QName. :)
(:*******************************************************:)
QName("example.com/", "p:ncname") le
					   QName("example.com/", "p:ncname")