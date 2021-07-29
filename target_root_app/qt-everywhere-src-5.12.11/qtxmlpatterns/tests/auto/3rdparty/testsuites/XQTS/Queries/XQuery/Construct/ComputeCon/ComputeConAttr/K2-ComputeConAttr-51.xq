(:*******************************************************:)
(: Test: K2-ComputeConAttr-51                            :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Construct an attribute with no prefix and a non-empty namespace URI. The specification is currently unclear on this area, http://www.w3.org/Bugs/Public/show_bug.cgi?id=4443 . :)
(:*******************************************************:)
<e>
    {
        attribute {QName("http://example.com/", "attr")} {()}
    }
</e>