(:*******************************************************:)
(: Test: function-declaration-021.xq          :)
(: Written By: Ravindranath Chennoju                   :)
(: Date: Wed Aug  31 13:13 2005                        :)
(: Purpose: Demonstrate function declaration - negative tests :)
(:********************************************************************:)

declare function local:foo ($n as xs:integer) as xs:string
{
   $n
};
local:foo(4)

