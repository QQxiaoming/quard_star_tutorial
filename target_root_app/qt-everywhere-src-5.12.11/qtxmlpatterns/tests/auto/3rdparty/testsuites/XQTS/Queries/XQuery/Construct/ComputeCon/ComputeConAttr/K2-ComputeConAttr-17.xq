(:*******************************************************:)
(: Test: K2-ComputeConAttr-17                            :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Ensure XQTY0024 is issued when content, set via a user declared function, appears before computed attributes. :)
(:*******************************************************:)
declare function local:myFunc() as item()
{
    <a/>
};

<b>
    {local:myFunc()}
    {attribute name {"content"}}
</b>