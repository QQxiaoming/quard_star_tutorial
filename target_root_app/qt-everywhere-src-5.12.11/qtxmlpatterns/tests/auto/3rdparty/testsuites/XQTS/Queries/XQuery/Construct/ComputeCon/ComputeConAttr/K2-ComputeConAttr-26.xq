(:*******************************************************:)
(: Test: K2-ComputeConAttr-26                            :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Add many attributes with a recursive user function. :)
(:*******************************************************:)
declare function local:myFunc()
{
    attribute name {"content"},
    <elem/>
};

<b>
    {local:myFunc()}
</b>