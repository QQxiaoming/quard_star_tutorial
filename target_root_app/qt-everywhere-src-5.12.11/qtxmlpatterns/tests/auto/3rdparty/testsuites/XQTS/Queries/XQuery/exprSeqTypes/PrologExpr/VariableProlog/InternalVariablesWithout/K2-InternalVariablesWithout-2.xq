(:*******************************************************:)
(: Test: K2-InternalVariablesWithout-2                   :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: A variable depending on a recursive function. :)
(:*******************************************************:)

declare variable $local:myVar := local:myFunction();
declare function local:myFunction()
{
        $local:myVar, 1, local:myFunction()
};
$local:myVar