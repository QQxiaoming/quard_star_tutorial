(:*******************************************************:)
(: Test: K2-InternalVariablesWithout-10                  :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Function arguments shadow global variables.  :)
(:*******************************************************:)
declare variable $local:myVar := local:myFunc(3);
declare function local:myFunc($local:myVar)
{
    $local:myVar
};
local:myFunc(6)