(:*******************************************************:)
(: Test: K2-ComputeConAttr-28                            :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Ensure XQTY0024 is issued when content, set via a user declared variable, appears before computed attributes. :)
(:*******************************************************:)
declare variable $local:myVar := <a/>;

<b>
    {$local:myVar}
    {attribute name {"content"}}
</b>