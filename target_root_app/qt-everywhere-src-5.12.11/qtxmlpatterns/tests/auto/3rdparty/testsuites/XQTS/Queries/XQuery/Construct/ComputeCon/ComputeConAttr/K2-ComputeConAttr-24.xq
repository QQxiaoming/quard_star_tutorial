(:*******************************************************:)
(: Test: K2-ComputeConAttr-24                            :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Ensure XQTY0024 is issued when content, set via a user declared variable, appears before computed attributes. :)
(:*******************************************************:)

declare variable $myVar := (attribute name {"content"}, <elem/>);
<b>
    {$myVar[2]}
</b>