(:*******************************************************:)
(: Test: K2-ComputeConAttr-54                            :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Ensure that a valid prefix has been constructed for a QName which the implementation is supposed to create a prefix for. :)
(:*******************************************************:)
string-length(xs:NCName(prefix-from-QName(node-name(attribute {QName("http://example.com/", "attr")} {()})))) > 0