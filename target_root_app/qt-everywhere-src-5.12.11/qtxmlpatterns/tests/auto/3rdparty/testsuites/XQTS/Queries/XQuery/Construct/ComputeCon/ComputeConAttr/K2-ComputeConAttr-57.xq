(:*******************************************************:)
(: Test: K2-ComputeConAttr-57                            :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Ensure the xmlns namespace is flagged as invalid even though no prefix is supplied. :)
(:*******************************************************:)
<e>
{
attribute {QName("http://www.w3.org/2000/xmlns/", "space")} {"default"}
}
</e>