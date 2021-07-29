(:*******************************************************:)
(: Test: K2-SeqExprCastable-1                            :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Testing castability to xs:QName where the cardinality is wrong. :)
(:*******************************************************:)
(QName("http://example.com/ANamespace", "ncname"),
 QName("http://example.com/ANamespace", "ncname2"),
 QName("http://example.com/ANamespace", "ncname3")) castable as xs:QName