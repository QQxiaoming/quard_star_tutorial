(:*******************************************************:)
(: Test: function-declaration-024.xq                     :)
(: Written By: Ravindranath Chennoju                     :)
(: Date: Wed Aug  31 13:13 2005                          :)
(: Purpose: Demonstrate function declaration - negative tests :)
(:*******************************************************:)

declare function local:foo ($n as xs:integer, $n as xs:integer)
{
 $n
};
local:foo(4, 1)

