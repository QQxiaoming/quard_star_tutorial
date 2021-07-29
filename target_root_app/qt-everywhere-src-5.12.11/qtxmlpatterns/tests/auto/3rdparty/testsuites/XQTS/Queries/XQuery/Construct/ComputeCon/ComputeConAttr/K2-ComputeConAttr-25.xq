(:*******************************************************:)
(: Test: K2-ComputeConAttr-25                            :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Add many attributes with a recursive user function. :)
(:*******************************************************:)
declare function local:myFunc($recurse as xs:integer)
{
attribute {concat("name", $recurse)} {"content"}
    ,
    if      ($recurse = 0)
    then    ()
    else    local:myFunc($recurse - 1)
};

<b>
    {local:myFunc(2)}
    {attribute name {"content"}}
</b>