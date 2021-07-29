(:*******************************************************:)
(: Test: function-declaration-023.xq                     :)
(: Written By: Ravindranath Chennoju                     :)
(: Date: Wed Aug  31 13:13 2005                          :)
(: Purpose: Demonstrate function declaration - negative tests :)
(:*******************************************************:)

declare function foo ($n as xs:integer)
{
   $n
};
foo(4)

