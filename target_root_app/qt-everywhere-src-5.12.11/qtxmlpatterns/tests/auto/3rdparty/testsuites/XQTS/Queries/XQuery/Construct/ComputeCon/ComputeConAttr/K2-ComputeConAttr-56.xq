(:*******************************************************:)
(: Test: K2-ComputeConAttr-56                            :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Ensure that the 'xml' prefix is used for the XML namespace when no prefix is supplied. :)
(:*******************************************************:)
<e>
{
    attribute {QName("http://www.w3.org/XML/1998/namespace", "space")} {"default"}
}
</e>