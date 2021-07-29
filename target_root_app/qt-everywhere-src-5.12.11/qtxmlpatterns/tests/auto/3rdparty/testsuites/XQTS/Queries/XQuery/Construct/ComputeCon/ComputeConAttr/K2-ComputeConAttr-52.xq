(:*******************************************************:)
(: Test: K2-ComputeConAttr-52                            :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Duplicated attributes, but constructed from different kinds of constructors. :)
(:*******************************************************:)
<e xmlns:p="http://example.com/" p:attr="">
    {
        attribute {QName("http://example.com/", "p:attr")} {()}
    }
</e>