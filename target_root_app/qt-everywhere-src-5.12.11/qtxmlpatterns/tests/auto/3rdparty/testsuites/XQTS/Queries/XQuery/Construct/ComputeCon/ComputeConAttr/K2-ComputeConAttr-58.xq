(:*******************************************************:)
(: Test: K2-ComputeConAttr-58                            :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Serialize an attribute that has a namespace URI but no prefix, and whose namespace URI is already in scope. :)
(:*******************************************************:)
<e xmlns:p="http://example.com/" p:attr1="value">
{
    attribute {QName("http://example.com/", "attr2")} {()}
}
</e>