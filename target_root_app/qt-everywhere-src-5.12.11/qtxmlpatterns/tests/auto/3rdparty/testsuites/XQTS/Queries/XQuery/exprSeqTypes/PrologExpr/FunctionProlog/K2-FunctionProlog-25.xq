(:*******************************************************:)
(: Test: K2-FunctionProlog-25                            :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: A call site that has an operand that gets treated as an xs:integer. :)
(:*******************************************************:)
declare function local:myFunc($recurse as xs:integer)
{
    attribute {concat("name", $recurse)} {()}
    ,
    if      ($recurse = 0)
    then    ()
    else    local:myFunc($recurse - 1)
};
<e>
    {
        local:myFunc((2, current-time())[1] treat as xs:integer)
    }
</e>