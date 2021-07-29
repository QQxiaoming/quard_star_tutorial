(:*******************************************************:)
(: Test: K-QNameEQ-7                                     :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Operator 'lt' is not available between values of type xs:QName. :)
(:*******************************************************:)
QName("example.com/", "p:ncname") lt
					   QName("example.com/", "p:ncname")