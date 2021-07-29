(:*******************************************************:)
(: Test: K2-ComputeConAttr-20                            :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Ensure XQTY0024 is issued when content, set via a user declared, recursive function, appears before computed attributes. :)
(:*******************************************************:)
declare function local:myFunc($recurse as xs:integer) as item()
{
    <nested>
        {
            if      ($recurse = 0)
            then    ()
            else    local:myFunc($recurse - 1)
        }
    </nested>
};

<b>
    {local:myFunc(3)}
    {attribute name {"content"}}
</b>