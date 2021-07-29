(:*******************************************************:)
(: Test: K2-ComputeConAttr-31                            :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Add an attribute from a function to an element. :)
(:*******************************************************:)
declare function local:myFunc()
{
    attribute n2 {"content"}
};
<b>
    {local:myFunc()}
    {attribute name {"content"}}
</b>