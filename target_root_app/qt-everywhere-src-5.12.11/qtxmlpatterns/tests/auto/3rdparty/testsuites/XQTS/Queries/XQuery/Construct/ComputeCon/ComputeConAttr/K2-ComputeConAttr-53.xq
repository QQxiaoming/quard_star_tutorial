(:*******************************************************:)
(: Test: K2-ComputeConAttr-53                            :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Serialize an attribute that has a namespace URI but no prefix. The implementation invents a prefix in this case. :)
(:*******************************************************:)
<e>
{
    attribute {QName("http://example.com/", "attr")} {()}
}
</e>