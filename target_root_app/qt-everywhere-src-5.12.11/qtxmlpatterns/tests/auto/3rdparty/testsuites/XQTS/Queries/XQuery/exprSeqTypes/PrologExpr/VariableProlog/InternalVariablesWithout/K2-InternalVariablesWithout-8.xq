(:*******************************************************:)
(: Test: K2-InternalVariablesWithout-8                   :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: A variable depending indirectly on a recursive function. :)
(:*******************************************************:)

declare variable $local:myVar := local:myFunction();
declare function local:myFunction2()
{
        local:myFunction4()
};
declare function local:myFunction4()
{
        local:myFunction2(), $local:myVar
};
declare function local:myFunction3()
{
        local:myFunction4()
};
declare function local:myFunction()
{
        local:myFunction3()
};
local:myFunction()
          