(:*******************************************************:)
(: Test: K-QNameEQ-5                                     :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Operator 'lt' is not available between xs:QName and xs:integer. :)
(:*******************************************************:)
QName("example.com/", "p:ncname") lt 1