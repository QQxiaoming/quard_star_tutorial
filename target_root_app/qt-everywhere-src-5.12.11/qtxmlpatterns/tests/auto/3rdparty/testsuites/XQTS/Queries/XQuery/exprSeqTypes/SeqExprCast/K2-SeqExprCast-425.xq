(:*******************************************************:)
(: Test: K2-SeqExprCast-425                              :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Serialize a set of xs:QName instances.       :)
(:*******************************************************:)
"START",
QName("http://example.com", "prefix:localName"),
QName("http://example.com", "localName"),
QName("http://example.com", "localName"),
QName("", "localName"),
QName((), "localName"),
"END"